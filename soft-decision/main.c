/*
*  Example demonstrating the different decoder implementations for soft decision decoding of convolutional codes
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "code.h"
#include "encoder.h"
#include "mapper.h"
#include "demapper.h"
#include "decoder.h"
#include "codebook.h"
#include "gaussian.h"

struct data {
    struct code_param parameter;
    struct encoder* encoder;
    struct mapper* mapper;
    struct demapper* demapper;
    struct decoder* decoder;
    uint64_t error_acc;
    float scaling;
    uint8_t wr_ptr;
    uint8_t data[265];
};

void print_data(uint8_t* data, uint8_t len) {
    for (uint8_t count = 0; count < len; ++count) {
        if (((data[count >> 3] << (count & 0x07)) & 0x80) == 0x80) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}

void print_tuples(float* data, uint8_t len, uint8_t num_elements) {
    uint8_t i;
    for (i = 0; i < len; i += num_elements) {
        printf("(");
        for (uint8_t k = 0; k < num_elements; ++k) {
            printf("%f ", data[i+k]);
        }
        printf(") ");
    }
    if (i != len) {
        printf("incpl.: (");
        for (i -= num_elements; i < len; ++i){
            printf("%f ",  data[i]);
        }
        printf(")");
    }
    printf("\n");
}

void print_code_params(struct code_param* param) {
    printf("Code Parameter:\n");
    printf("Symlen out:             %1d\n", param->symlen_out);
    printf("Constraint length:      %1d\n", param->constr_len);
    printf("Block length:          %02d\n", param->block_len);
    for (int i = 0; i < param->symlen_out; ++i) {
        printf("P%1d:", i);
        for (int k = 0; k < (22 - param->constr_len); ++k) {
            printf(" ");
        }
        for (int k = 0; k < param->constr_len; ++k) {
            if (((param->polynomials[i] << k) & 0x8000000000000000) == 0x8000000000000000) {
                printf("1");
            } else {
                printf("0");
            }
        }
        printf("\n");
    }
}

int data_callback_encoded(uint8_t* data, uint8_t len, void* userdata) {
    struct data* pdata = (struct data*) userdata;

    //printf("data encoded: ");
    //for (uint8_t i = 0; i < len; ++i) {
    //    printf("%d ", data[i]);
    //}
    //printf("\n");

    (void) mapper_input(pdata->mapper, data, len);
    return -1; // delete data
}

int data_callback_mapped(float* data, uint8_t len, void* userdata) {
    struct data* pdata = (struct data*) userdata;

    //printf("data mapped: ");
    //print_tuples(data, len, 2);

    // add gaussian noise
    for (uint8_t i = 0; i < len; ++i) {
        data[i] += pdata->scaling * pregenerated[rand() % num_elements_pregenerated];
    }

    (void) demapper_input(pdata->demapper, data, len);
    return -1;
}

int data_callback_demapped(float* data, uint8_t len, void* userdata) {
    struct data* pdata = (struct data*) userdata;

    //printf("data demapped: ");
    //print_tuples(data, len, 1 << pdata->parameter.symlen_out);

    (void) decoder_input(pdata->decoder, data, len);
    return -1;
}

int data_callback_decoded(uint8_t* data, uint8_t len, void* userdata) {
    struct data* pdata = (struct data*) userdata;

    //printf("data decoded: ");
    //print_data(data, len);

    if (pdata->wr_ptr != (len + 7) / 8) {
        printf("Error: input data len: %d decoded length: %d\n", pdata->wr_ptr, (len / 8) + 1);
    }
    pdata->wr_ptr = 0;

    uint16_t nBitErrors = 0;
    for (int i = 0; i < len; ++i) {
        int index = i / 8;
        int shift = i % 8;

        uint8_t data_decoded = (data[index] << shift) & 0x80;
        uint8_t data_in      = (pdata->data[index] << shift) & 0x80;
        if (data_in != data_decoded) {
            nBitErrors++;
        }
    }

    pdata->error_acc += nBitErrors;

    return -1; // delete data
}

int main(void) {
    srand(time(NULL)); // use current time as seed for random generator

    float snr_db_values[] = {
        0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0
    };
    // scaling factors are computed using following formula:
    // s = 10^(-SNRdb/20)
    float noise_scaling_factors[] = {
        1.0000, 0.7943, 0.6310, 0.5012, 0.3981, 0.3162, 0.2512, 0.1995, 0.1585, 0.1259, 0.1000
    };
    int num_scaling_factors = sizeof(noise_scaling_factors) / sizeof(noise_scaling_factors[0]);

    struct data userdata;

    get_code(0, &(userdata.parameter));

    userdata.encoder  = encoder_create();
    userdata.mapper   = mapper_create();
    userdata.demapper = demapper_create();
    userdata.decoder  = decoder_create();
    userdata.parameter.userdata = (void*) &userdata;
    userdata.wr_ptr = 0;
    userdata.scaling = 1.0;

    (void) encoder_init(userdata.encoder, &(userdata.parameter));
    encoder_register_callback(userdata.encoder, data_callback_encoded);

    (void) mapper_init(userdata.mapper, &(userdata.parameter));
    mapper_register_callback(userdata.mapper, data_callback_mapped);

    (void) demapper_init(userdata.demapper, &(userdata.parameter));
    demapper_register_callback(userdata.demapper, data_callback_demapped);

    (void) decoder_init(userdata.decoder, &(userdata.parameter));
    decoder_register_callback(userdata.decoder, data_callback_decoded);

    print_code_params(&(userdata.parameter));
    for (int i = 0; i < num_scaling_factors; ++i) {
        userdata.scaling = noise_scaling_factors[i];

        printf("====================================================\n");
        printf("Channel SNR: %fdB\n", snr_db_values[i]);

        userdata.error_acc = 0;

        int num_bits_transmitted = 0;
        // 800.000.000 bit = 16.000.000 * 50 bit
        // 800.000.000 bit = 20.000.000 * 40 bit
        int numblocks = 20000000;
        if (snr_db_values[i] <= 6.0) {
            numblocks = 200000;
        } else if (snr_db_values[i] <= 12.0) {
            numblocks = 2000000;
        }
        for (int k = 0; k < numblocks; ++k) {
            if (k % 1024 == 0) {
                printf("Calculating... %3d%%\r", (k*100)/numblocks);
                fflush(stdout);
            }
            for (int j = 0; j < ((userdata.parameter.block_len + 7) / 8); ++j) {
                userdata.data[(userdata.wr_ptr)++] = rand() % 256;
            }

            //printf("data: ");
            //print_data(userdata.data, userdata.parameter.block_len);
            (void) encoder_input(userdata.encoder, userdata.data, userdata.parameter.block_len);
            num_bits_transmitted += userdata.parameter.block_len;

            if(userdata.wr_ptr != 0) {
                printf("Error: userdata wr pointer is not reset.\n");
            }
        }
        printf("\n");

        // error analysis
        printf("Overall transmission error rate: %.8lf\n", ((double)userdata.error_acc)/((double)num_bits_transmitted));
    }

    encoder_destroy(&(userdata.encoder));
    mapper_destroy(&(userdata.mapper));
    demapper_destroy(&(userdata.demapper));
    decoder_destroy(&(userdata.decoder));

    return 0;
}
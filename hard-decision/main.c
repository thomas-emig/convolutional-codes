/*
*  Example demonstrating the different decoder implementations for decoding of convolutional codes
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "code.h"
#include "encoder.h"
#include "decoder.h"
#include "codebook.h"

struct data {
    struct encoder* encoder;
    struct decoder* decoder;
    int32_t channel_error_rate; // The transmission error probability in 10^(-6)
    uint8_t symbol_length;
    uint64_t metric_acc;
    uint64_t error_acc;
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
    struct decoder* decoder = pdata->decoder;

    // introcude some transmission errors
    for (uint8_t i = 0; i < len; ++i) {
        for (uint8_t k = 0;  k < pdata->symbol_length; ++k) {
            if ((rand() % 1000000) < pdata->channel_error_rate) {
                data[i] ^= (1 << k);
            }
        }
    }

    (void) decoder_input(decoder, data, len);
    return -1; // delete data
}

int data_callback_decoded(uint8_t* data, uint8_t len, uint16_t path_metric, void* userdata) {
    struct data* pdata = (struct data*) userdata;

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

    pdata->metric_acc += path_metric;
    pdata->error_acc += nBitErrors;

    return -1; // delete data
}

int main(void) {
    srand(time(NULL)); // use current time as seed for random generator

    int channel_error_rates[] = {
             1,      5,     25,    125,
           625,   3125,   6250,  12500,
         15625,  25000,  50000,  78125,
        100000, 200000, 300000, 390625,
        400000
    };
    int num_error_rates = sizeof(channel_error_rates) / sizeof(int);

    struct code_param param;
    get_code(0, &param);

    struct encoder* encoder = encoder_create();
    struct decoder* decoder = decoder_create();

    struct data userdata = {
        .encoder = encoder,
        .decoder = decoder,
        .symbol_length = param.symlen_out,
        .metric_acc = 0,
        .error_acc = 0,
        .wr_ptr = 0
    };

    param.userdata = (void*) &userdata;
    (void) encoder_init(encoder, &param);
    encoder_register_callback(encoder, data_callback_encoded);

    (void) decoder_init(decoder, &param);
    decoder_register_callback(decoder, data_callback_decoded);

    print_code_params(&param);
    for (int i = 0; i < num_error_rates; ++i) {
        userdata.channel_error_rate = channel_error_rates[i];

        printf("====================================================\n");
        printf("Channel error rate: %lf\n", ((double)userdata.channel_error_rate)/(1e6));

        userdata.error_acc = 0;
        userdata.metric_acc = 0;

        int num_bits_transmitted = 0;
        // 800.000.000 bit = 16.000.000 * 50 bit
        // 800.000.000 bit = 20.000.000 * 40 bit
        int numblocks = 20000000;
        if (userdata.channel_error_rate > 12500) {
            numblocks /= 10;
        }
        if (userdata.channel_error_rate > 50000) {
            numblocks /= 10;
        }
        if (userdata.channel_error_rate > 200000) {
            numblocks /= 10;
        }
        for (int k = 0; k < numblocks; ++k) {
            if (k % 128 == 0) {
                printf("Calculating... %3d%%\r", (k*100)/numblocks);
                fflush(stdout);
            }
            for (int j = 0; j < ((param.block_len + 7) / 8); ++j) {
                userdata.data[(userdata.wr_ptr)++] = rand() % 256;
            }

            (void) encoder_input(encoder, userdata.data, param.block_len);
            num_bits_transmitted += param.block_len;

            if(userdata.wr_ptr != 0) {
                printf("Error: userdata wr pointer is not reset.\n");
            }
        }
        printf("\n");

        // error analysis
        printf("Overall transmission error rate: %.8lf\n", ((double)userdata.error_acc)/((double)num_bits_transmitted));
    }

    encoder_destroy(&encoder);
    decoder_destroy(&decoder);

    return 0;
}

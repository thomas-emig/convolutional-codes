/*
*  Example demonstrating uncoded data transmission on the AWGN channel
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "code.h"
#include "mapper.h"
#include "demapper.h"
#include "codebook.h"
#include "gaussian.h"

struct data {
    struct code_param parameter;
    struct mapper* mapper;
    struct demapper* demapper;
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

int data_callback_mapped(float* data, uint8_t len, void* userdata) {
    struct data* pdata = (struct data*) userdata;

    //printf("data mapped: ");
    //print_tuples(data, len, 2);

    // add gaussian noise
    for (uint8_t i = 0; i < len; ++i) {
        data[i] += pdata->scaling * gengauss();
    }

    (void) demapper_input(pdata->demapper, data, len);
    return -1;
}

int data_callback_demapped(float* data, uint8_t len, void* userdata) {
    struct data* pdata = (struct data*) userdata;

    if (len % (1 << pdata->parameter.symlen_out) != 0) {
        printf("error: demapper did not output a complete set of distances.\n");
        return -1;
    }
    pdata->wr_ptr = 0;

    //printf("data demapped: ");
    //print_tuples(data, len, 1 << pdata->parameter.symlen_out);

    float min_dist = INFINITY;
    uint8_t min_idx = 0;
    for (uint8_t i = 0; i < (1 << pdata->parameter.symlen_out); ++i) {
        if (data[i] < min_dist) {
            min_dist = data[i];
            min_idx = i;
        }
    }

    // decoded symbol is now in min_idx - tx symbol is in pdata->data[0]
    uint8_t dist = min_idx ^ pdata->data[0];
    dist = (dist & 0x55) + ((dist >> 1) & 0x55);
    dist = (dist & 0x33) + ((dist >> 2) & 0x33);
    dist = (dist & 0x0F) + ((dist >> 4) & 0x0F);

    pdata->error_acc += dist;

    return -1;
}

int main(void) {
    srand(time(NULL)); // use current time as seed for random generator

    // Eb/N0 in dB
    float snr_db_values[] = {
        0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0//, 16.0, 18.0, 20.0
    };
    // scaling factors are computed using following formula:
    // s = 1/sqrt(2) * 10^(-SNRdb/20) // SNRdb is the bit energy to noise ratio Eb/N0
    float noise_scaling_factors[] = {
        0.707106781186548, 0.561674881261479, 0.446154216921401, 0.354392891541971,
        0.281504279937367, 0.223606797749979, 0.177617192929090, 0.141086351316046,
        //0.112068872384565, 0.089019469568772, 0.070710678118655
    };
    int num_scaling_factors = sizeof(noise_scaling_factors) / sizeof(noise_scaling_factors[0]);

    struct data userdata;

    get_code(0, &(userdata.parameter));

    userdata.mapper   = mapper_create();
    userdata.demapper = demapper_create();
    userdata.parameter.userdata = (void*) &userdata;
    userdata.wr_ptr = 0;
    userdata.scaling = 1.0;

    // convert from Es/N0 to Eb/N0
    for (int i = 0; i < num_scaling_factors; ++i) {
        noise_scaling_factors[i] *= 1.0 / sqrt(userdata.parameter.symlen_out);
    }

    (void) mapper_init(userdata.mapper, &(userdata.parameter));
    mapper_register_callback(userdata.mapper, data_callback_mapped);

    (void) demapper_init(userdata.demapper, &(userdata.parameter));
    demapper_register_callback(userdata.demapper, data_callback_demapped);

    print_code_params(&(userdata.parameter));
    for (int i = 0; i < num_scaling_factors; ++i) {
        userdata.scaling = noise_scaling_factors[i];

        printf("====================================================\n");
        printf("Eb/N0: %fdB\n", snr_db_values[i]);

        userdata.error_acc = 0;

        int num_bits_transmitted = 0;
        int numbits    = 800000000;
        int numsymbols = numbits / userdata.parameter.symlen_out;
        for (int k = 0; k < numsymbols; ++k) {

            userdata.data[(userdata.wr_ptr)++] = rand() % (1 << userdata.parameter.symlen_out);

            (void) mapper_input(userdata.mapper, userdata.data, 1);
            num_bits_transmitted += userdata.parameter.symlen_out;

            if(userdata.wr_ptr != 0) {
                printf("Error: userdata wr pointer is not reset.\n");
            }
        }

        // error analysis
        printf("Overall transmission error rate: %.8lf\n", ((double)userdata.error_acc)/((double)num_bits_transmitted));
    }

    mapper_destroy(&(userdata.mapper));
    demapper_destroy(&(userdata.demapper));

    return 0;
}

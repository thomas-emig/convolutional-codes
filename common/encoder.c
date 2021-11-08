/*
*  Implementation of an encoder for convolutional codes
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "code.h"
#include "encoder.h"

struct encoder {
    struct code_param parameter;
    uint64_t shift_register;
    uint8_t output_data_len;
    uint8_t output_counter;
    uint8_t* output_data;
    int (*output_callback) (uint8_t* data, uint8_t len, void* userdata);
};

struct encoder* encoder_create(void) {
    struct encoder* res = (struct encoder*) malloc(sizeof(struct encoder));
    if (res != NULL) {
        memset(res, 0, sizeof(struct encoder));
    }
    return res;
}

void encoder_destroy(struct encoder** obj) {
    if ((*obj)->output_data != NULL) {
        free((*obj)->output_data);
        (*obj)->output_data = NULL;
    }

    if (*obj != NULL) {
        free(*obj);
        *obj = NULL;
    }
}

int encoder_init(struct encoder* obj, struct code_param* param) {
    obj->parameter = *param;
    obj->output_data_len = obj->parameter.block_len + obj->parameter.constr_len - 1;

    if (obj->output_data != NULL) {
        free(obj->output_data);
        obj->output_data = NULL;
    }
    obj->output_data = (uint8_t*) malloc(obj->output_data_len * sizeof(uint8_t));
    if (obj->output_data == NULL) {
        return -1;
    }

    return encoder_reset(obj);
}

void encoder_register_callback(struct encoder* obj, int (*output_callback) (uint8_t* data, uint8_t len, void* userdata)) {
    obj->output_callback = output_callback;
}

int encoder_reset(struct encoder* obj) {
    if (obj->output_counter == obj->parameter.block_len + obj->parameter.constr_len - 1) {
        int res = -1;
        if (obj->output_callback != NULL) {
            res = obj->output_callback(obj->output_data, obj->output_data_len, obj->parameter.userdata);
        }
        if (res == 0) {
            obj->output_data = NULL;
            obj->output_data = (uint8_t*) malloc(obj->output_data_len * sizeof(uint8_t));
            if (obj->output_data == NULL) {
                return -1;
            }
        }
    }

    memset(obj->output_data, 0, obj->output_data_len * sizeof(uint8_t));

    obj->output_counter = 0;
    obj->shift_register = 0;

    return 0;
}

int encoder_input(struct encoder* obj, uint8_t* data, uint8_t len) {
    for (uint8_t count = 0; count < len; ++count) {

        uint64_t current_bit = (data[count / 8] << (count % 8)) & 0x80;
        obj->shift_register >>= 1;
        obj->shift_register |= current_bit << 56;

        obj->output_data[obj->output_counter] = 0;
        for (uint8_t n = 0; n < obj->parameter.symlen_out; ++n) {
            uint64_t val = obj->shift_register & obj->parameter.polynomials[n];

            // calculate parity
            val ^= val >> 32;
            val ^= val >> 16;
            val ^= val >> 8;
            val ^= (val >> 4) & 0x0F;
            uint8_t res = (0x6996 >> val) & 1;

            obj->output_data[obj->output_counter] |= res;
            if (n < (obj->parameter.symlen_out - 1)) {
                obj->output_data[obj->output_counter] <<= 1;
            }
        }
        obj->output_counter++;

        if (obj->output_counter == obj->parameter.block_len + obj->parameter.constr_len - 1) {
            return encoder_reset(obj);
        }
        if (obj->output_counter == obj->parameter.block_len) {
            uint8_t dummy[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            return encoder_input(obj, dummy, obj->parameter.constr_len - 1);
        }
    }

    return 0;
}

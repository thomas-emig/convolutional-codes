/*
*  Implementation of the famous soft decision viterbi decoding for convolutional codes
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "code.h"
#include "decoder.h"

struct state {
    uint16_t prev_idx;
    uint8_t input;
};

struct decoder {
    struct code_param parameter;
    uint16_t num_states;
    uint16_t num_symbols;
    uint16_t symbol_counter;
    struct state* state_matrix;
    float* state_metrics;
    uint8_t* output_data;
    callback_t output_callback;
};

static void get_transition_metric(struct decoder* obj,
                                  uint16_t current_state,
                                  uint8_t encoder_input,
                                  float* rx_metrics,
                                  uint16_t* new_state,
                                  float* transition_metric);
static void traceback(struct decoder* obj);
static int emit_output_buffer(struct decoder* obj);
static int receive_symbol(struct decoder* obj, float* symbol_metrics);

static void get_transition_metric(struct decoder* obj,
                                  uint16_t current_state,
                                  uint8_t encoder_input,
                                  float* rx_metrics,
                                  uint16_t* new_state,
                                  float* transition_metric) {

    uint64_t shift_register = ((uint64_t) current_state) << (64 - obj->parameter.constr_len);
    shift_register |= ((uint64_t)encoder_input) << 63;

    uint8_t expected_symbol = 0;
    for (uint8_t n = 0; n < obj->parameter.symlen_out; ++n) {
        uint64_t val = shift_register & obj->parameter.polynomials[n];

        // calculate parity
        val ^= val >> 32;
        val ^= val >> 16;
        val ^= val >> 8;
        val ^= (val >> 4) & 0x0F;
        uint8_t res = (0x6996 >> val) & 1;

        expected_symbol |= res;
        if (n != obj->parameter.symlen_out - 1) {
            expected_symbol <<= 1;
        }
    }

    *new_state = current_state >> 1;
    *new_state |= encoder_input << (obj->parameter.constr_len - 2);

    *transition_metric = rx_metrics[expected_symbol];
}

static void traceback(struct decoder* obj) {
    uint16_t current_state_index = 0;
    float min_metric = INFINITY;
    for (uint8_t i = 0; i < obj->num_states; ++i) {
        if (obj->state_metrics[i] < min_metric) {
            min_metric = obj->state_metrics[i];
            current_state_index = i;
        }
    }

    while (obj->symbol_counter != 0) {
        obj->symbol_counter--;
        struct state* current_states = &(obj->state_matrix[obj->num_states*obj->symbol_counter]);

        uint8_t decoded_symbol = current_states[current_state_index].input;
        current_state_index = current_states[current_state_index].prev_idx;

        obj->output_data[obj->symbol_counter / 8] |= decoded_symbol << (7 - (obj->symbol_counter % 8));
    }
}

static int emit_output_buffer(struct decoder* obj) {
    int res = -1;
    if (obj->output_callback != NULL) {
        res = obj->output_callback(obj->output_data, obj->parameter.block_len, obj->parameter.userdata);
    }
    if (res == 0) {
        obj->output_data = NULL;
        uint8_t output_buf_len = (obj->num_symbols + 7) / 8; // round up
        obj->output_data = (uint8_t*) malloc(output_buf_len * sizeof(uint8_t));
        if (obj->output_data == NULL) {
            return -1;
        }
    }
    return 0;
}

static int receive_symbol(struct decoder* obj, float* symbol_metrics) {
    float new_metrics[obj->num_states];
    for (uint16_t s = 0; s < obj->num_states; ++s) {
        new_metrics[s] = INFINITY;
    }

    struct state* current_states = &(obj->state_matrix[obj->num_states*obj->symbol_counter]);
    for (uint16_t s = 0; s < obj->num_states; ++s) {
        for(uint8_t i = 0; i < 2; ++i) {
            uint16_t new_state;
            float transition_metric;
            get_transition_metric(obj, s, i, symbol_metrics, &new_state, &transition_metric);

            float new_state_metric = obj->state_metrics[s] + transition_metric;
            if (new_state_metric < new_metrics[new_state]) {
                new_metrics[new_state] = new_state_metric;
                current_states[new_state].prev_idx = s;
                current_states[new_state].input = i;
            }
        }
    }

    memcpy(obj->state_metrics, new_metrics, sizeof(new_metrics));

    obj->symbol_counter++;
    if (obj->symbol_counter == obj->num_symbols) {
        traceback(obj);
        if (emit_output_buffer(obj) != 0) {
            return -1;
        }
        decoder_reset(obj);
    }
    return 0;
}

// API
struct decoder* decoder_create(void) {
    struct decoder* res = (struct decoder*) malloc(sizeof(struct decoder));
    if (res != NULL) {
        memset(res, 0, sizeof(struct decoder));
    }
    return res;
}

void decoder_destroy(struct decoder** obj) {
    if ((*obj)->state_matrix != NULL) {
        free((*obj)->state_matrix);
        (*obj)->state_matrix = NULL;
    }
    if ((*obj)->state_metrics != NULL) {
        free((*obj)->state_metrics);
        (*obj)->state_metrics = NULL;
    }

    if (*obj != NULL) {
        free(*obj);
        *obj = NULL;
    }
}

int decoder_init(struct decoder* obj, struct code_param* param) {
    obj->parameter = *param;

    if (obj->state_matrix != NULL) {
        free(obj->state_matrix);
        obj->state_matrix = NULL;
    }
    if (obj->state_metrics != NULL) {
        free(obj->state_metrics);
        obj->state_metrics = NULL;
    }

    obj->num_states = 1 << (obj->parameter.constr_len - 1);
    obj->num_symbols = obj->parameter.block_len + obj->parameter.constr_len - 1;

    obj->state_matrix = (struct state*) malloc((obj->num_states * obj->num_symbols) * sizeof(struct state));
    if (obj->state_matrix == NULL) {
        return -1;
    }
    memset(obj->state_matrix, 0, (obj->num_states * obj->num_symbols) * sizeof(struct state));

    obj->state_metrics = (float*) malloc(obj->num_states * sizeof(float));
    if (obj->state_metrics == NULL) {
        return -1;
    }

    if (obj->output_data != NULL) {
        free(obj->output_data);
        obj->output_data = NULL;
    }
    uint8_t output_buf_len = (obj->num_symbols + 7) / 8; // round up
    obj->output_data = (uint8_t*) malloc(output_buf_len * sizeof(uint8_t));
    if (obj->output_data == NULL) {
        return -1;
    }

    decoder_reset(obj);
    return 0;
}

void decoder_register_callback(struct decoder* obj, callback_t output_callback) {
    obj->output_callback = output_callback;
}

void decoder_reset(struct decoder* obj) {
    obj->symbol_counter = 0;

    obj->state_metrics[0] = 0.0;
    for (uint16_t s = 1; s < obj->num_states; ++s) {
        obj->state_metrics[s] = INFINITY;
    }

    uint8_t output_buf_len = (obj->num_symbols + 7) / 8; // round up
    memset(obj->output_data, 0, output_buf_len * sizeof(uint8_t));
}

int decoder_input(struct decoder* obj, float* data, uint8_t len) {
    uint8_t num_symbols = 1 << obj->parameter.symlen_out;
    if (len % num_symbols != 0) {
        return -1;
    }
    for (uint8_t count = 0; count < len; count += num_symbols) {
        int res = receive_symbol(obj, &(data[count]));
        if (res != 0) {
            return res;
        }
    }
    return 0;
}

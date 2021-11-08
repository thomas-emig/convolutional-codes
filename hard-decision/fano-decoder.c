/*
*  Implementation of the fano algorithm for decoding of convolutional codes
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "code.h"
#include "decoder.h"

struct decoder {
    struct code_param parameter;
    int (*output_callback) (uint8_t* data, uint8_t len, uint16_t path_metric, void* userdata);

    uint8_t* input_symbols;
    uint8_t input_symbol_write_index;

    uint8_t next_decoding_symbol;
    uint8_t* decoded_path;
    uint64_t* states;
    int32_t* metrics;
    int32_t metric_threshold;

    uint8_t bufsize; // output buffer size

    int32_t delta;
};

static int receive_symbol(struct decoder* obj, uint8_t received_symbol);
static void check_and_delete(void** obj);
static void get_transition_metric(struct decoder* obj,
                                  uint64_t current_state,
                                  uint8_t received_symbol,
                                  uint64_t* new_state,
                                  int32_t* transition_metric);

// API
struct decoder* decoder_create(void) {
    struct decoder* res = (struct decoder*) malloc(sizeof(struct decoder));
    if (res != NULL) {
        memset(res, 0, sizeof(*res));
    }
    return res;
}

void decoder_destroy(struct decoder** obj) {
    check_and_delete((void**)&((*obj)->input_symbols));
    check_and_delete((void**)&((*obj)->decoded_path));
    check_and_delete((void**)&((*obj)->states));
    check_and_delete((void**)&((*obj)->metrics));
    check_and_delete((void**)obj);
}

void decoder_register_callback(struct decoder* obj, int (*output_callback) (uint8_t* data, uint8_t len, uint16_t path_metric, void* userdata)) {
    obj->output_callback = output_callback;
}

int decoder_init(struct decoder* obj, struct code_param* param) {
    obj->parameter = *param;
    uint8_t blocklen_w_tail = obj->parameter.block_len + obj->parameter.constr_len - 1;
    obj->bufsize = (blocklen_w_tail + 7) / 8;
    

    check_and_delete((void**)&(obj->input_symbols));
    obj->input_symbols = malloc(blocklen_w_tail * sizeof(uint8_t));
    if (obj->input_symbols == NULL) {
        return -1;
    }

    check_and_delete((void**)&(obj->decoded_path));
    obj->decoded_path = malloc(obj->bufsize * sizeof(uint8_t));
    if (obj->decoded_path == NULL) {
        return -1;
    }

    check_and_delete((void**)&(obj->states));
    obj->states = malloc(blocklen_w_tail * sizeof(uint64_t));
    if (obj->states == NULL) {
        return -1;
    }

    check_and_delete((void**)&(obj->metrics));
    obj->metrics = malloc(blocklen_w_tail * sizeof(int32_t));
    if (obj->metrics == NULL) {
        return -1;
    }

    obj->delta = 3;

    decoder_reset(obj);
    return 0;
}

void decoder_reset(struct decoder* obj) {
    // reset input symbol buffer
    obj->input_symbol_write_index = 0;

    // reset threshold
    obj->metric_threshold = 0;

    // reset path memory
    memset(obj->decoded_path, 0, obj->bufsize);
    obj->next_decoding_symbol = 0;
}

int decoder_input(struct decoder* obj, uint8_t* data, uint8_t len) {
    uint8_t symbol_mask = (1 << obj->parameter.symlen_out) - 1;
    for (uint8_t count = 0; count < len; ++count) {
        int res = receive_symbol(obj, data[count] & symbol_mask);
        if (res != 0) {
            return res;
        }
    }
    return 0;
}

// local functions
static int receive_symbol(struct decoder* obj, uint8_t received_symbol) {
    // add received symbol to buffer
    obj->input_symbols[obj->input_symbol_write_index++] = received_symbol;

    uint8_t traced_back = 0;
    while (obj->next_decoding_symbol != obj->input_symbol_write_index) {
        // compute new metrics
        uint64_t current_state = 0;
        if (obj->next_decoding_symbol - 1 >= 0) {
            current_state = obj->states[obj->next_decoding_symbol - 1];
        }

        uint64_t new_states[2];
        int32_t transition_metrics[2];
        get_transition_metric(
            obj,
            current_state,
            obj->input_symbols[obj->next_decoding_symbol],
            new_states,
            transition_metrics
        );

        // select successor path
        uint8_t selected_index = 0;
        if (transition_metrics[1] > transition_metrics[0]) {
            selected_index = 1;
        }

        int32_t current_metric = 0;
        if (obj->next_decoding_symbol - 1 >= 0) {
            current_metric = obj->metrics[obj->next_decoding_symbol - 1];
        }

        uint8_t found_path = 1;
        if (traced_back == 1) {
            // ignore the path we came from
            if (new_states[selected_index] == obj->states[obj->next_decoding_symbol]) {
                selected_index ^= 1; // 1 -> 0, 0 -> 1, choose the other path
            }

            // ensure that new path has a lower or equal to the path we came from
            if ((current_metric + transition_metrics[selected_index]) > obj->metrics[obj->next_decoding_symbol]) {
                found_path = 0;
            }
        }

        int32_t metric_suc = current_metric + transition_metrics[selected_index];
        uint64_t state_suc = new_states[selected_index];

        if ((found_path == 1) && (metric_suc >= obj->metric_threshold)) {
            // move forward
            uint8_t outbuf_widx = obj->next_decoding_symbol++;
            obj->states[outbuf_widx] = state_suc;
            obj->metrics[outbuf_widx] = metric_suc;
            obj->decoded_path[outbuf_widx / 8] |= selected_index << (7 - (outbuf_widx % 8));
            traced_back = 0;

            // is current path a complete code path?
            if (obj->next_decoding_symbol == obj->parameter.block_len + obj->parameter.constr_len - 1) {
                int res = -1;
                if (obj->output_callback != NULL) {
                    res = obj->output_callback(
                        obj->decoded_path,
                        obj->parameter.block_len,
                        obj->metrics[obj->next_decoding_symbol - 1],
                        obj->parameter.userdata
                    );
                }

                // if client took ownership of data, we need to get a new buffer
                if (res == 0) {
                    obj->decoded_path = NULL;
                    obj->decoded_path = malloc(obj->bufsize * sizeof(uint8_t));
                    if (obj->decoded_path == NULL) {
                        return -1;
                    }
                }

                decoder_reset(obj);
                return 0;
            }

            // tightening
            if (obj->next_decoding_symbol - 2 >= 0) {
                if (obj->metrics[obj->next_decoding_symbol - 2] < (obj->metric_threshold + obj->delta)) {
                    int32_t m = (obj->metrics[obj->next_decoding_symbol - 1] - obj->metric_threshold) / obj->delta;
                    obj->metric_threshold += m * obj->delta;
                }
            }

        } else {
            if ((obj->next_decoding_symbol - 2 >= 0) && (obj->metrics[obj->next_decoding_symbol - 2] >= obj->metric_threshold)) {
                // move backward
                uint8_t outbuf_widx = --(obj->next_decoding_symbol);
                obj->decoded_path[outbuf_widx / 8] &= ~(1 << (7 - (outbuf_widx % 8)));
                traced_back = 1;
            } else {
                // lower the threshold
                obj->metric_threshold -= obj->delta;
                traced_back = 0;
            }
        }
    }

    return 0;
}

static void check_and_delete(void** obj) {
    if (*obj != NULL) {
        free(*obj);
        *obj = NULL;
    }
}

static void get_transition_metric(struct decoder* obj,
                                  uint64_t current_state,
                                  uint8_t received_symbol,
                                  uint64_t* new_state,
                                  int32_t* transition_metric) {

    uint64_t shift_register = current_state << (64 - obj->parameter.constr_len);
    shift_register |= (uint64_t)1 << 63;

    for (uint8_t in = 0; in < 2; ++in) {
        shift_register ^= (uint64_t)1 << 63;

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

        new_state[in] = shift_register >> (64 + 1 - obj->parameter.constr_len);

        // calculate hamming distance between received symbol and expected symbol
        transition_metric[in] = expected_symbol ^ received_symbol;
        transition_metric[in] = (transition_metric[in] & 0x55) + ((transition_metric[in] >> 1) & 0x55);
        transition_metric[in] = (transition_metric[in] & 0x33) + ((transition_metric[in] >> 2) & 0x33);
        transition_metric[in] = (transition_metric[in] & 0x0F) + ((transition_metric[in] >> 4) & 0x0F);

        transition_metric[in] = (transition_metric[in] * obj->parameter.bit_metrics[1]) + ((obj->parameter.symlen_out - transition_metric[in]) * obj->parameter.bit_metrics[0]);
    }
}

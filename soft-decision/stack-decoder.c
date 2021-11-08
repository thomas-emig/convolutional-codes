/*
*  Implementation of the stack algorithm for decoding of convolutional codes
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "code.h"
#include "decoder.h"

#define STACK_DEPTH         (64) // maximum number of paths we can store

struct path {
    uint16_t next_input_index; // the next input symbol we would like to use for decoding of this path
    uint64_t encoder_state;    // the encoder state at the end of this path
    float metric;              // metric of this path
    uint8_t* decoded_path;     // the actual path
};

struct decoder {
    struct code_param parameter;
    int (*output_callback) (uint8_t* data, uint8_t len, void* userdata);

    float* input_symbols; // stores blocklen (inc. tail) * numsymbols distance values to the actually received symbols
    uint16_t input_symbol_write_index;

    struct path path_stack[STACK_DEPTH];
    uint16_t next_stack_idx;

    uint8_t bufsize; // output buffer size
};

static int receive_symbol(struct decoder* obj, float* received_symbol);
static uint16_t get_most_probable_path(struct decoder* obj);
static uint16_t get_least_probable_path(struct decoder* obj);
static uint16_t duplicate_path(struct decoder* obj, uint16_t rhs);
static void get_transition_metric(struct decoder* obj,
                                  uint64_t current_state,
                                  float* received_symbol,
                                  uint64_t* new_state,
                                  float* transition_metric);

// API
struct decoder* decoder_create(void) {
    struct decoder* res = (struct decoder*) malloc(sizeof(struct decoder));
    if (res != NULL) {
        memset(res, 0, sizeof(*res));
    }
    return res;
}

void decoder_destroy(struct decoder** obj) {
    if ((*obj)->input_symbols != NULL) {
        free((*obj)->input_symbols);
        (*obj)->input_symbols = NULL;
    }
    for (uint16_t i = 0; i < STACK_DEPTH; ++i) {
        if ((*obj)->path_stack[i].decoded_path != NULL) {
            free((*obj)->path_stack[i].decoded_path);
            (*obj)->path_stack[i].decoded_path = NULL;
        }
    }
    if (*obj != NULL) {
        free(*obj);
        *obj = NULL;
    }
}

void decoder_register_callback(struct decoder* obj, int (*output_callback) (uint8_t* data, uint8_t len, void* userdata)) {
    obj->output_callback = output_callback;
}

int decoder_init(struct decoder* obj, struct code_param* param) {
    obj->parameter = *param;

    if(obj->input_symbols != NULL) {
        free(obj->input_symbols);
        obj->input_symbols = NULL;
    }
    uint8_t num_symbols = 1 << obj->parameter.symlen_out;
    obj->input_symbols = malloc((obj->parameter.block_len + obj->parameter.constr_len - 1) * num_symbols * sizeof(float));
    if (obj->input_symbols == NULL) {
        return -1;
    }

    // pre-allocate stack
    obj->bufsize = (obj->parameter.block_len + obj->parameter.constr_len - 1 + 7) / 8;
    for (uint16_t i = 0; i < STACK_DEPTH; ++i) {
        if (obj->path_stack[i].decoded_path != NULL) {
            free(obj->path_stack[i].decoded_path);
            obj->path_stack[i].decoded_path = NULL;
        }
        obj->path_stack[i].decoded_path = malloc(obj->bufsize * sizeof(uint8_t));
        if (obj->path_stack[i].decoded_path == NULL) {
            return -1;
        }
        memset(obj->path_stack[i].decoded_path, 0, obj->bufsize);
    }

    decoder_reset(obj);
    return 0;
}

void decoder_reset(struct decoder* obj) {
    // reset input symbol buffer
    obj->input_symbol_write_index = 0;

    // reset stack
    obj->next_stack_idx = 1;
    obj->path_stack[0].next_input_index = 0;
    obj->path_stack[0].encoder_state = 0;
    obj->path_stack[0].metric = 0.0;
    memset(obj->path_stack[0].decoded_path, 0, obj->bufsize);
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

// local functions
static int receive_symbol(struct decoder* obj, float* received_symbol) {
    // add received symbol to buffer
    uint8_t num_symbols = 1 << obj->parameter.symlen_out;
    memcpy(&(obj->input_symbols[obj->input_symbol_write_index]), received_symbol, num_symbols * sizeof(float));
    obj->input_symbol_write_index += num_symbols;

    uint16_t current_path = get_most_probable_path(obj);
    while (obj->path_stack[current_path].next_input_index != obj->input_symbol_write_index) {
        // compute new metrics
        uint64_t new_states[2];
        float transition_metrics[2];
        get_transition_metric(
            obj,
            obj->path_stack[current_path].encoder_state,
            &(obj->input_symbols[obj->path_stack[current_path].next_input_index]),
            new_states,
            transition_metrics
        );

        // duplicate path to follow both paths
        uint32_t new_path = duplicate_path(obj, current_path);

        // extend both paths by current transition
        uint8_t selected_index = 0;
        uint8_t outbuf_widx = obj->path_stack[current_path].next_input_index / num_symbols;
        obj->path_stack[current_path].next_input_index += num_symbols;
        obj->path_stack[current_path].encoder_state = new_states[selected_index];
        obj->path_stack[current_path].metric += transition_metrics[selected_index];
        obj->path_stack[current_path].decoded_path[outbuf_widx / 8] |= selected_index << (7 - (outbuf_widx % 8));

        selected_index = 1;
        outbuf_widx = obj->path_stack[new_path].next_input_index / num_symbols;
        obj->path_stack[new_path].next_input_index += num_symbols;
        obj->path_stack[new_path].encoder_state = new_states[selected_index];
        obj->path_stack[new_path].metric += transition_metrics[selected_index];
        obj->path_stack[new_path].decoded_path[outbuf_widx / 8] |= selected_index << (7 - (outbuf_widx % 8));

        // select new path to follow
        current_path = get_most_probable_path(obj);
    }

    // if we received all symbols of this block, output decoded sequence and reset decoder
    if ((obj->input_symbol_write_index / num_symbols) == obj->parameter.block_len + obj->parameter.constr_len - 1) {
        int res = -1;
        if (obj->output_callback != NULL) {
            res = obj->output_callback(
                obj->path_stack[current_path].decoded_path,
                obj->parameter.block_len,
                obj->parameter.userdata
            );
        }

        // if client took ownership of data, we need to get a new buffer
        if (res == 0) {
            obj->path_stack[current_path].decoded_path = NULL;
            obj->path_stack[current_path].decoded_path = malloc(obj->bufsize * sizeof(uint8_t));
            if (obj->path_stack[current_path].decoded_path == NULL) {
                return -1;
            }
        }

        decoder_reset(obj);
    }

    return 0;
}

static uint16_t get_least_probable_path(struct decoder* obj) {
    uint16_t min_idx = obj->next_stack_idx;
    float min_metric = INFINITY;

    for (uint16_t i = 0; i < obj->next_stack_idx; ++i) {
        if (obj->path_stack[i].metric < min_metric) {
            min_metric = obj->path_stack[i].metric;
            min_idx = i;
        }
    }

    return min_idx;
}

static uint16_t get_most_probable_path(struct decoder* obj) {
    uint16_t max_idx = obj->next_stack_idx;
    float max_metric = -INFINITY;

    for (uint16_t i = 0; i < obj->next_stack_idx; ++i) {
        if (obj->path_stack[i].metric > max_metric) {
            max_metric = obj->path_stack[i].metric;
            max_idx = i;
        }
    }

    return max_idx;
}

static uint16_t duplicate_path(struct decoder* obj, uint16_t rhs) {
    uint16_t lhs;
    if (obj->next_stack_idx == STACK_DEPTH) {
        lhs = get_least_probable_path(obj);
    } else {
        lhs = obj->next_stack_idx++;
    }

    obj->path_stack[lhs].next_input_index = obj->path_stack[rhs].next_input_index;
    obj->path_stack[lhs].encoder_state    = obj->path_stack[rhs].encoder_state;
    obj->path_stack[lhs].metric           = obj->path_stack[rhs].metric;
    memcpy(obj->path_stack[lhs].decoded_path, obj->path_stack[rhs].decoded_path, obj->bufsize);

    return lhs;
}

static void get_transition_metric(struct decoder* obj,
                                  uint64_t current_state,
                                  float* received_symbol,
                                  uint64_t* new_state,
                                  float* transition_metric) {

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

        transition_metric[in] = 1.0 + obj->parameter.metric_weight * received_symbol[expected_symbol];
    }
}

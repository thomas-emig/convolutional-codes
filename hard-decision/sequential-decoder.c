/*
*  Implementation of a sequential tree decoder with traceback ability for decoding of convolutional codes.
*/

#include <stdlib.h>
#include <string.h>

#include "code.h"
#include "decoder.h"

#define MAX_TRACEBACK_DEPTH 6
#define MAX_NODES 65534
#define INITIAL_METRIC_TRESHOLD 3

struct node {
    uint64_t     state;
    uint32_t     metric;
    struct node* parent;
    struct node* children[2];
    uint8_t      children_visited[2];
};

struct decoder {
    struct code_param parameter;
    int (*output_callback) (uint8_t* data, uint8_t len, uint16_t path_metric, void* userdata);

    struct node root[MAX_NODES];
    uint16_t next_free_node_idx;

    struct node* current_node;
    uint32_t metric_threshold;

    uint8_t* input_symbols;
    uint8_t input_symbol_write_index;

    uint8_t* output_buffer;
};

static int receive_symbol(struct decoder* obj, uint8_t received_symbol, uint8_t traceback_mode);
static struct node* alloc_node(struct decoder* obj);
static void decode_from_tree(struct decoder* obj);
static int emit_output_buffer(struct decoder* obj, uint32_t path_metric);
static void get_transition_metric(struct decoder* obj,
                                  uint64_t current_state,
                                  uint8_t received_symbol,
                                  uint64_t* new_state,
                                  uint32_t* transition_metric);

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
    if ((*obj)->output_buffer != NULL) {
        free((*obj)->output_buffer);
        (*obj)->output_buffer = NULL;
    }
    if (*obj != NULL) {
        free(*obj);
        *obj = NULL;
    }
}

void decoder_register_callback(struct decoder* obj, int (*output_callback) (uint8_t* data, uint8_t len, uint16_t path_metric, void* userdata)) {
    obj->output_callback = output_callback;
}

int decoder_init(struct decoder* obj, struct code_param* param) {
    obj->parameter = *param;

    if(obj->input_symbols != NULL) {
        free(obj->input_symbols);
        obj->input_symbols = NULL;
    }
    obj->input_symbols = malloc((obj->parameter.block_len + obj->parameter.constr_len - 1) * sizeof(uint8_t));
    if (obj->input_symbols == NULL) {
        return -1;
    }

    if(obj->output_buffer != NULL) {
        free(obj->output_buffer);
        obj->output_buffer = NULL;
    }
    size_t output_bufsize = (obj->parameter.block_len + obj->parameter.constr_len - 1 + 7) / 8;
    obj->output_buffer = malloc(output_bufsize * sizeof(uint8_t));
    if (obj->output_buffer == NULL) {
        return -1;
    }
    memset(obj->output_buffer, 0, output_bufsize);

    decoder_reset(obj);
    return 0;
}

void decoder_reset(struct decoder* obj) {
    // reset decoding tree
    memset(obj->root, 0, MAX_NODES * sizeof(struct node));
    obj->next_free_node_idx = 1;
    obj->current_node = obj->root;

    // reset traceback system
    obj->metric_threshold = INITIAL_METRIC_TRESHOLD;

    // reset input symbol buffer
    obj->input_symbol_write_index = 0;
}

int decoder_input(struct decoder* obj, uint8_t* data, uint8_t len) {
    uint8_t symbol_mask = (1 << obj->parameter.symlen_out) - 1;
    for (uint8_t count = 0; count < len; ++count) {
        int res = receive_symbol(obj, data[count] & symbol_mask, 0);
        if (res != 0) {
            return res;
        }
    }
    return 0;
}

// local functions
static int receive_symbol(struct decoder* obj, uint8_t received_symbol, uint8_t traceback_mode) {
    // add received symbol to buffer
    if (traceback_mode == 0) {
        obj->input_symbols[obj->input_symbol_write_index++] = received_symbol;
    }

    // if we already calculated child metrics and allocated nodes we do not need to repeat this process
    size_t child_index = 0;
    if ((obj->current_node->children[0] == NULL) || (obj->current_node->children[1] == NULL)) {
        // determine states and transition metrics for child nodes
        uint32_t child_metrics[2];
        uint64_t child_states[2];
        get_transition_metric(obj, obj->current_node->state, received_symbol, child_states, child_metrics);

        // decide for lower metric
        if (child_metrics[1] < child_metrics[0]) {
            child_index = 1;
        }

        // allocate new nodes and insert into tree
        for (size_t i = 0; i < 2; ++i) {
            if (obj->current_node->children[i] == NULL) {
                obj->current_node->children[i] = alloc_node(obj);
                if (obj->current_node->children[i] == NULL) {
                    return -1;
                }
            }
            obj->current_node->children[i]->parent = obj->current_node;
            obj->current_node->children[i]->state = child_states[i];
            obj->current_node->children[i]->metric = obj->current_node->metric + child_metrics[i];
            obj->current_node->children_visited[i] = 0;
        }
    } else {
        for (size_t i = 0; i < 2; ++i) {
            obj->current_node->children_visited[i] = 0;
        }

        if (obj->current_node->children[1]->metric < obj->current_node->children[0]->metric) {
            child_index = 1;
        }
    }

    if (obj->current_node->children[child_index]->metric <= obj->metric_threshold) {
        // set visited and advance to child with lower metric
        obj->current_node->children_visited[child_index] = 1;
        obj->current_node = obj->current_node->children[child_index];
    } else { // both metrics are above threshold
        obj->current_node->children_visited[0] = 1;
        obj->current_node->children_visited[1] = 1;

        // trace back to find path with lower metric
        uint8_t symbols_backtraced = 1;
        uint8_t done = 0;
        while (done == 0) {
            for (size_t i = 0; i < 2; ++i) {
                if (obj->current_node->children_visited[i] == 0) {
                    obj->current_node->children_visited[i] = 1;
                    if (obj->current_node->children[i]->metric <= obj->metric_threshold) {
                        obj->current_node = obj->current_node->children[i];
                        symbols_backtraced--;
                        done = 1;
                        break;
                    }
                }
            }
            if (done == 0) {
                if ((obj->current_node != obj->root) && (symbols_backtraced < MAX_TRACEBACK_DEPTH)) {
                    obj->current_node = obj->current_node->parent;
                    symbols_backtraced++;
                } else {
                    obj->metric_threshold++;
                    done = 1;
                }
            }
        }

        // trace forward backtraced symbols
        for (uint8_t rd_idx = obj->input_symbol_write_index - symbols_backtraced; rd_idx < obj->input_symbol_write_index; ++rd_idx) {
            receive_symbol(obj, obj->input_symbols[rd_idx], 1);
        }
    }

    if(obj->input_symbol_write_index == (obj->parameter.block_len + obj->parameter.constr_len - 1) && (traceback_mode == 0)) {
        uint32_t path_metric = obj->current_node->metric;
        decode_from_tree(obj);
        if (emit_output_buffer(obj, path_metric) != 0) {
            return -1;
        }
        decoder_reset(obj);
    }

    return 0;
}

static struct node* alloc_node(struct decoder* obj) {
    if (obj->next_free_node_idx == MAX_NODES) {
        return NULL;
    }
    return &(obj->root[obj->next_free_node_idx++]);
}

static void decode_from_tree(struct decoder* obj)
{
    uint8_t buffer_write_index = obj->parameter.block_len + obj->parameter.constr_len - 2;

    while (obj->current_node != obj->root) {
        struct node* old_node = obj->current_node;
        obj->current_node = obj->current_node->parent;

        uint8_t i;
        for (i = 0; i < 1; ++i) {
            if (old_node == obj->current_node->children[i]) {
                break;
            }
        }

        obj->output_buffer[buffer_write_index / 8] |= i << (7 - (buffer_write_index % 8));
        buffer_write_index--;
    }
}

static int emit_output_buffer(struct decoder* obj, uint32_t path_metric) {
    int res = -1;
    if (obj->output_callback != NULL) {
        res = obj->output_callback(obj->output_buffer, obj->parameter.block_len, path_metric, obj->parameter.userdata);
    }
    // res != 0: callback did not take ownership of data -> we can reuse ist
    // res == 0: callback took ownership of data -> we need to allocate a new buffer
    size_t output_bufsize = (obj->parameter.block_len + obj->parameter.constr_len - 1 + 7) / 8;
    if (res == 0) {
        obj->output_buffer = NULL;
        obj->output_buffer = malloc(output_bufsize * sizeof(uint8_t));
        if (obj->output_buffer == NULL) {
            return -1;
        }
    }
    memset(obj->output_buffer, 0, output_bufsize);

    return 0;
}

static void get_transition_metric(struct decoder* obj,
                                  uint64_t current_state,
                                  uint8_t received_symbol,
                                  uint64_t* new_state,
                                  uint32_t* transition_metric) {

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
    }
}

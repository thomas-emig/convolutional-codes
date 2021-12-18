/*
 *  Implementation of the fano algorithm for decoding of convolutional codes
 *  This decoder is derived from the fano decoder implementation by Phil Karn KA9Q
 *  which can be found at http://www.ka9q.net/code/fec/
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "code.h"
#include "decoder.h"

#define TIMEOUT 10000 // decoder timeout in cycles per decoded bit
#define DELTA   17    // stepsize for threshold adjustment

struct node {
    uint64_t state;
    int32_t metric;
    uint8_t selected_path;         // the (currently) best path (either 0 or 1)
    int32_t transition_metrics[2]; // transition metrics for input 0 and 1
    uint64_t successor_states[2];  // states of the successor nodes for input 0 and 1
    uint8_t decoded_input;         // the (currently) best guess for the input bit
    uint8_t received_symbol;       // the rx symbol for the corresponding node
};

struct decoder {
    struct code_param parameter;
    int (*output_callback) (uint8_t* data, uint8_t len, uint16_t path_metric, void* userdata);

    // fano parameter
    struct node* current_node; // currently examined symbol
    int32_t metric_threshold;
    struct node* nodes;        // the examined nodes
    int32_t delta;
    uint8_t blocklen_w_tail;
    uint32_t timeout;
    uint8_t input_counter;
    bool input_ignore;         // ignore input in case of timeout

    // output buffer
    uint8_t bufsize;           // output buffer size
    uint8_t* decoded_path;
};

static int receive_symbol(struct decoder* obj, uint8_t received_symbol);
static void check_and_delete(void** obj);
static void get_transition_metric(struct decoder* obj,
                                  uint64_t current_state,
                                  uint8_t received_symbol,
                                  uint64_t* new_state,
                                  int32_t* transition_metric);
static int output_data(struct decoder* obj);

// API
struct decoder* decoder_create(void) {
    struct decoder* res = (struct decoder*) malloc(sizeof(struct decoder));
    if (res != NULL) {
        memset(res, 0, sizeof(*res));
    }
    return res;
}

void decoder_destroy(struct decoder** obj) {
    check_and_delete((void**)&((*obj)->decoded_path));
    check_and_delete((void**)&((*obj)->nodes));
    check_and_delete((void**)obj);
}

void decoder_register_callback(struct decoder* obj, int (*output_callback) (uint8_t* data, uint8_t len, uint16_t path_metric, void* userdata)) {
    obj->output_callback = output_callback;
}

int decoder_init(struct decoder* obj, struct code_param* param) {
    obj->parameter = *param;
    obj->blocklen_w_tail = obj->parameter.block_len + obj->parameter.constr_len - 1;
    obj->bufsize = (obj->blocklen_w_tail + 7) / 8;

    check_and_delete((void**)&(obj->decoded_path));
    obj->decoded_path = malloc(obj->bufsize * sizeof(uint8_t));
    if (obj->decoded_path == NULL) {
        return -1;
    }

    check_and_delete((void**)&(obj->nodes));
    obj->nodes = malloc(obj->blocklen_w_tail * sizeof(struct node));
    if (obj->nodes == NULL) {
        return -1;
    }

    obj->delta = DELTA;

    decoder_reset(obj);
    return 0;
}

void decoder_reset(struct decoder* obj) {
    // reset threshold
    obj->metric_threshold = 0;
    obj->timeout = TIMEOUT * obj->blocklen_w_tail;
    obj->input_counter = 0;
    obj->input_ignore = false;

    // reset path memory
    memset(obj->decoded_path, 0, obj->bufsize);

    // reset node memory
    memset(obj->nodes, 0, obj->blocklen_w_tail * sizeof(struct node));
    obj->current_node = obj->nodes;
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
    obj->input_counter++;
    if (obj->input_ignore) {
        if (obj->input_counter == obj->blocklen_w_tail) {
            return output_data(obj);
        }
        return 0;
    }

    struct node* cnode = obj->current_node;
    struct node* nextnode = obj->current_node + 1;

    cnode->received_symbol = received_symbol;

    // compute transition metrics
    get_transition_metric(
        obj,
        cnode->state,
        cnode->received_symbol,
        cnode->successor_states,
        cnode->transition_metrics
    );

    // sort transition metrics and successor states
    if (cnode->transition_metrics[0] < cnode->transition_metrics[1]) {
        // swap values
        uint64_t state_tmp = cnode->successor_states[0];
        cnode->successor_states[0] = cnode->successor_states[1];
        cnode->successor_states[1] = state_tmp;

        int32_t metric_tmp = cnode->transition_metrics[0];
        cnode->transition_metrics[0] = cnode->transition_metrics[1];
        cnode->transition_metrics[1] = metric_tmp;

        cnode->decoded_input = 1;
    }

    while (obj->timeout != 0) {
        obj->timeout--;

        // compute successor metric
        int32_t ms = cnode->metric + cnode->transition_metrics[cnode->selected_path];

        if (ms >= obj->metric_threshold) {
            // tightening
            if (cnode->metric < (obj->metric_threshold + obj->delta)) {
                while (ms >= obj->metric_threshold + obj->delta) {
                    obj->metric_threshold += obj->delta;
                }
            }

            // move forward
            obj->current_node++;
            if (obj->current_node == obj->nodes + obj->blocklen_w_tail) {
                // all block symbols are decoded
                return output_data(obj);
            }
            obj->current_node->state = cnode->successor_states[cnode->selected_path];
            obj->current_node->metric = ms;
            cnode = obj->current_node;

            // did we decode all currently available symbols?
            if (cnode == nextnode) {
                return 0;
            }

            // recalculate transition metrics because the state of the current node changed
            get_transition_metric(
                obj,
                cnode->state,
                cnode->received_symbol,
                cnode->successor_states,
                cnode->transition_metrics
            );

            cnode->decoded_input = 0;
            cnode->selected_path = 0;

            // sort transition metrics and successor states
            if (cnode->transition_metrics[0] < cnode->transition_metrics[1]) {
                // swap values
                uint64_t state_tmp = cnode->successor_states[0];
                cnode->successor_states[0] = cnode->successor_states[1];
                cnode->successor_states[1] = state_tmp;

                int32_t metric_tmp = cnode->transition_metrics[0];
                cnode->transition_metrics[0] = cnode->transition_metrics[1];
                cnode->transition_metrics[1] = metric_tmp;

                cnode->decoded_input = 1;
            }
        } else {
            while (1) {
                struct node* pnode = cnode - 1;

                // if we can not move back, relax threshold
                if ((cnode == obj->nodes) || (pnode->metric < obj->metric_threshold)) {
                    obj->metric_threshold -= obj->delta;

                    // start again with search from path 0
                    if (cnode->selected_path != 0) {
                        cnode->selected_path = 0;
                        cnode->decoded_input ^= 1; // swap decoded input bit as we are now choosing the other path
                    }
                    break;
                } else {
                    // move back
                    obj->current_node = pnode;
                    cnode = pnode;

                    // if all branches already tested, move further back, else test next best branch
                    if (cnode->selected_path == 0) { // cnode->selected_path < 1
                        cnode->selected_path = 1;
                        cnode->decoded_input ^= 1; // swap decoded input bit as we are now choosing the other path
                        break;
                    }
                }
            }
        }
    }

    // reached timeout - ignore further input until block length reached
    if (obj->input_counter == obj->blocklen_w_tail) {
        return output_data(obj);
    }
    obj->input_ignore = true;
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

    for (uint8_t in = 0; in < 2; ++in) {
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

        transition_metric[in] = (transition_metric[in] * obj->parameter.fano_bit_metrics[1]) + ((obj->parameter.symlen_out - transition_metric[in]) * obj->parameter.fano_bit_metrics[0]);

        shift_register |= (uint64_t)1 << 63;
    }
}

static int output_data(struct decoder* obj) {
    struct node* enode = obj->nodes + obj->blocklen_w_tail;
    uint8_t output_idx = 0;
    for (struct node* cnode = obj->nodes; cnode != enode; ++cnode) {
        if (cnode->decoded_input == 1) {
            uint8_t byte_idx = output_idx / 8;
            uint8_t shift = 7 - (output_idx % 8);
            obj->decoded_path[byte_idx] |= (uint8_t)1 << shift;
        }
        output_idx++;
    }

    // emit output data
    int res = -1;
    if (obj->output_callback != NULL) {
        res = obj->output_callback(
            obj->decoded_path,
            obj->parameter.block_len,
            0, // TODO: return metric
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

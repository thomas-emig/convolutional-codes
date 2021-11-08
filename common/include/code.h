/*
*  Definition of code parameter for convolutional codes.
*/

#pragma once

#include <stdint.h>

struct code_param {
    uint8_t symlen_out;
    uint8_t constr_len;
    uint8_t block_len;
    uint64_t* polynomials;
    int32_t* bit_metrics; // 0: metric correct bit, 1: metric wrong bit - used for hard-decision stack decoding
    float metric_weight;  // weight factor for squared distance metric used by soft-decision stack decoder
    void* userdata;
};

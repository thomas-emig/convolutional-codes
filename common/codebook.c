#include <stddef.h>

#include "codebook.h"
#include "code.h"

// The bit metric usually depends on the channel error probability
// The metrics given here were found by experiment. By keeping the metics
// constant, we introduce an additional error to the decoding system which
// is not too large but should be mentioned.
// Additionally, for soft-decision decoding, we introduce an error by not using
// the real metric but rather a function depending on the quadratic
// distance to the expected symbol.

static uint64_t polynomials1[] = {
    0xA000000000000000, // P0 = (1,0,1)
    0x6000000000000000  // P1 = (0,1,1)
};
static int32_t metrics1[] = {1, -15};
static int32_t fmetrics1[] = {1, -20};

static uint64_t polynomials2[] = {
    0xB000000000000000, // P0 = (1,0,1,1)
    0xE000000000000000  // P1 = (1,1,1,0)
};
static int32_t metrics2[] = {1, -25};
static int32_t fmetrics2[] = {1, -45};

static uint64_t polynomials3[] = {
    0xA800000000000000, // P0 = (1,0,1,0,1)
    0xF000000000000000, // P1 = (1,1,1,1,0)
};
static int32_t metrics3[] = {1, -30};
static int32_t fmetrics3[] = {1, -48};

static uint64_t polynomials4[] = {
    0xB400000000000000, // P0 = (1,0,1,1,0,1)
    0xE800000000000000, // P1 = (1,1,1,0,1,0)
};
static int32_t metrics4[] = {1, -39};
static int32_t fmetrics4[] = {1, -55};

// the wspr code (k = 32)
static uint64_t polynomials5[] = {
    0x8ACA0B4F00000000, // P0 = (10001010110010100000101101001111)
    0xE23C862700000000, // P1 = (11100010001111001000011000100111)
};
static int32_t metrics5[] = {1, -27};
static int32_t fmetrics5[] = {1, -38};

static uint64_t polynomials6[] = {
    0xA000000000000000, // P0 = (1,0,1)
    0xC000000000000000, // P1 = (1,1,0)
    0x2000000000000000  // P2 = (0,0,1)
};
static int32_t metrics6[] = {1, -9};
static int32_t fmetrics6[] = {1, -30};

static uint8_t symlen[] = {
    2, 2, 2, 2, 2, 3
};

static uint8_t constraintlength[] = {
    3, 4, 5, 6, 32, 3
};

static uint8_t blocklength[] = {
    40, 40, 40, 40, 50, 40
};

static float metric_weigths[] = {
    -6.5, -14.5, -14.0, -12.5, -6.0, -10.0
};

static uint64_t* polynomials[] = {
    polynomials1,
    polynomials2,
    polynomials3,
    polynomials4,
    polynomials5,
    polynomials6
};

static int32_t* bit_metrics[] = {
    metrics1,
    metrics2,
    metrics3,
    metrics4,
    metrics5,
    metrics6
};

static int32_t* fmetrics[] = {
    fmetrics1,
    fmetrics2,
    fmetrics3,
    fmetrics4,
    fmetrics5,
    fmetrics6
};

void get_code(uint8_t index, struct code_param* parameter) {
    parameter->symlen_out = symlen[index];
    parameter->constr_len = constraintlength[index];
    parameter->block_len = blocklength[index];
    parameter->polynomials = polynomials[index];
    parameter->bit_metrics = bit_metrics[index];
    parameter->fano_bit_metrics = fmetrics[index];
    parameter->metric_weight = metric_weigths[index];
    parameter->userdata = NULL;
}

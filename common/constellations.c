#include <stdint.h>
#include <stddef.h>

#include "constellations.h"

// QAM constellations with gray code bit mapping and unit signal power

static float c_1[] = {
     0.5,  0.5,
    -0.5, -0.5,
};

static float c_2[] = {
     0.5,  0.5,
     0.5, -0.5,
    -0.5,  0.5,
    -0.5, -0.5,
};

static float c_3[] = {
     0.167,  0.167,  0.167,  0.5  ,
    -0.167,  0.167, -0.5  ,  0.167,
     0.167, -0.167,  0.5  , -0.167,
    -0.167, -0.167, -0.167, -0.5  ,
};

static float* constellations[] = {
    NULL,
    c_1,
    c_2,
    c_3,
};

float* get_constellation(uint8_t num_bits) {
    return constellations[num_bits];
}

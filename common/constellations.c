#include <stdint.h>
#include <stddef.h>

#include "constellations.h"

// QAM constellations with gray code bit mapping and unit signal power: abs(x)^2 == 1

static float c_1[] = {
     0.707107,  0.707107,
    -0.707107, -0.707107,
};

static float c_2[] = {
     0.707107,  0.707107,
     0.707107, -0.707107,
    -0.707107,  0.707107,
    -0.707107, -0.707107,
};

static float c_3[] = {
     0.408248,  0.408248,  0.408248,  1.224745,
    -0.408248,  0.408248, -1.224745,  0.408248,
     0.408248, -0.408248,  1.224745, -0.408248,
    -0.408248, -0.408248, -0.408248, -1.224745,
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

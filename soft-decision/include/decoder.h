/*
*  API of a convolutional soft decision decoder
*/

#pragma once

#include <stdint.h>

struct code_param;

// an opaque struct to hold the decoder itself
struct decoder;

// constuctor/destructor
struct decoder* decoder_create(void);
void decoder_destroy(struct decoder** obj);

// bring the decoder into a known state
int decoder_init(struct decoder* obj, struct code_param* param);
void decoder_register_callback(struct decoder* obj, int (*output_callback) (uint8_t* data, uint8_t len, void* userdata));
void decoder_reset(struct decoder* obj);

// feed data into the decoder, len is the data size in float words
int decoder_input(struct decoder* obj, float* data, uint8_t len);

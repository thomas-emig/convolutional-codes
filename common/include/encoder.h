/*
*  API of a decoder for convolutional codes.
*/

#pragma once

#include <stdint.h>

struct code_param;

// an opaque struct to hold the encoder itself
struct encoder;

// constuctor/destructor
struct encoder* encoder_create(void);
void encoder_destroy(struct encoder** obj);

// bring the encoder into a known state
int encoder_init(struct encoder* obj, struct code_param* param);
void encoder_register_callback(struct encoder* obj, int (*output_callback) (uint8_t* data, uint8_t len, void* userdata));
int encoder_reset(struct encoder* obj);

// feed data into the encoder, len is the data size in *bits*
int encoder_input(struct encoder* obj, uint8_t* data, uint8_t len);

/*
*  API of a convolutional decoder
*/

#pragma once

#include <stdint.h>

typedef int (*callback_t) (uint8_t* data, uint8_t len, int32_t path_metric, void* userdata);

struct code_param;

// an opaque struct to hold the decoder itself
struct decoder;

// constuctor/destructor
struct decoder* decoder_create(void);
void decoder_destroy(struct decoder** obj);

// bring the decoder into a known state
int decoder_init(struct decoder* obj, struct code_param* param);
void decoder_register_callback(struct decoder* obj, callback_t output_callback);
void decoder_reset(struct decoder* obj);

// feed data into the decoder, len is the data size in *bits*
int decoder_input(struct decoder* obj, uint8_t* data, uint8_t len);

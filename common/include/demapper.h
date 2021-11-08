/*
 *  Interface for the soft demapping of received symbols
 */

#pragma once

#include <stdint.h>

struct demapper;
struct code_param;

struct demapper* demapper_create(void);
void demapper_destroy(struct demapper** obj);

int demapper_init(struct demapper* obj, struct code_param* param);
void demapper_register_callback(struct demapper* obj, int (*output_callback) (float* data, uint8_t len, void* userdata));

int demapper_input(struct demapper* obj, float* data, uint8_t len);

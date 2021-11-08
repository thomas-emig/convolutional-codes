/*
 *  Interface for the mapping of symbol bits to baseband values
 */

#pragma once

#include <stdint.h>

struct mapper;
struct code_param;

struct mapper* mapper_create(void);
void mapper_destroy(struct mapper** obj);

int mapper_init(struct mapper* obj, struct code_param* param);
void mapper_register_callback(struct mapper* obj, int (*output_callback) (float* data, uint8_t len, void* userdata));

int mapper_input(struct mapper* obj, uint8_t* data, uint8_t len);

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "mapper.h"
#include "code.h"
#include "constellations.h"

struct mapper {
    float* constellation;
    float* output_data;
    void* userdata;
    int (*output_callback) (float* data, uint8_t len, void* userdata);
};

struct mapper* mapper_create(void) {
    struct mapper* res = (struct mapper*) malloc(sizeof(struct mapper));
    if (res != NULL) {
        memset(res, 0, sizeof(struct mapper));
    }
    return res;
}

void mapper_destroy(struct mapper** obj) {
    if (*obj != NULL) {
        if ((*obj)->output_data != NULL) {
            free((*obj)->output_data);
            (*obj)->output_data = NULL;
        }

        free (*obj);
        *obj = NULL;
    }
}

int mapper_init(struct mapper* obj, struct code_param* param) {
    if (obj->output_data == NULL) {
        obj->output_data = (float*) malloc(2 * sizeof(float));
    }

    if (obj->output_data == NULL) {
        return -1;
    }

    obj->constellation = get_constellation(param->symlen_out);
    obj->userdata = param->userdata;
    return 0;
}

void mapper_register_callback(struct mapper* obj, int (*output_callback) (float* data, uint8_t len, void* userdata)) {
    obj->output_callback = output_callback;
}

int mapper_input(struct mapper* obj, uint8_t* data, uint8_t len) {
    for (uint8_t i = 0; i < len; ++i) {
        memcpy(obj->output_data, &(obj->constellation[2 * data[i]]), 2 * sizeof(float));

        int res = -1;
        if (obj->output_callback != NULL) {
            res = obj->output_callback(obj->output_data, 2, obj->userdata);
        }
        if (res == 0) {
            obj->output_data = NULL;
            obj->output_data = (float*) malloc(2 * sizeof(float));
            if (obj->output_data == NULL) {
                return -1;
            }
        }
    }
    return 0;
}

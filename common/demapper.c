#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "demapper.h"
#include "code.h"
#include "constellations.h"

struct demapper {
    float* constellation;
    float* output_data;
    uint8_t output_data_len;
    void* userdata;
    int (*output_callback) (float* data, uint8_t len, void* userdata);
};

struct demapper* demapper_create(void) {
    struct demapper* res = (struct demapper*) malloc(sizeof(struct demapper));
    if (res != NULL) {
        memset(res, 0, sizeof(struct demapper));
    }
    return res;
}
void demapper_destroy(struct demapper** obj) {
    if (*obj != NULL) {
        if ((*obj)->output_data != NULL) {
            free((*obj)->output_data);
            (*obj)->output_data = NULL;
        }

        free (*obj);
        *obj = NULL;
    }
}

int demapper_init(struct demapper* obj, struct code_param* param) {
    obj->output_data_len = 1 << param->symlen_out;
    obj->constellation = get_constellation(param->symlen_out);
    obj->userdata = param->userdata;

    if (obj->output_data == NULL) {
        obj->output_data = (float*) malloc(obj->output_data_len * sizeof(float));
    }

    if (obj->output_data == NULL) {
        return -1;
    }
    return 0;
}

void demapper_register_callback(struct demapper* obj, int (*output_callback) (float* data, uint8_t len, void* userdata)) {
    obj->output_callback = output_callback;
}

int demapper_input(struct demapper* obj, float* data, uint8_t len) {
    if (len % 2 != 0) {
        return -1;
    }
    for (uint8_t i = 0; i < len; i += 2) {
        for (uint8_t k = 0; k < obj->output_data_len; ++k) {
            float dx = data[i] - obj->constellation[2*k];
            float dy = data[i+1] - obj->constellation[(2*k)+1];
            obj->output_data[k] = (dx*dx) + (dy*dy);
        }

        int res = -1;
        if (obj->output_callback != NULL) {
            res = obj->output_callback(obj->output_data, obj->output_data_len, obj->userdata);
        }
        if (res == 0) {
            obj->output_data = NULL;
            obj->output_data = (float*) malloc(obj->output_data_len * sizeof(float));
            if (obj->output_data == NULL) {
                return -1;
            }
        }
    }
    return 0;
}

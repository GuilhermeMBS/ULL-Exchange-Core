#include <stdbool.h>
#include <stdint.h>

#include "errorlib.h"
#include "book.h"
#include "validator.h"

void vld_validate_order(obk_order_t *buffer, int32_t count) {
    if (!buffer || count <= 0) return;

    for (int32_t i = 0; i < count; i++) {
        obk_order_t *o = &buffer[i];

        if (!o->is_valid) {
            o->order_id = (uint32_t)-1;
            continue;
        }

        bool valid = true;

        if (o->price <= 0.0) valid = false;

        if (valid && o->quantity < 1) valid = false;

        if (valid && o->side != 'A' && o->side != 'B') valid = false;

        if (!valid) {
            o->is_valid = false;
            o->order_id = (uint32_t)-1;
        }
    }
}

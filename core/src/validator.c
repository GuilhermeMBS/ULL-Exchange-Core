#include <stdbool.h>
#include <stdint.h>

#include "retcodes.h"
#include "book.h"
#include "validator.h"


ret_code_t vld_validate_order(obk_order_t order, bool *out_is_valid) {
    if (!out_is_valid) return ERR_ORD;

    if (!order.is_valid) {
        *out_is_valid = false;
        return ERR_NONE;
    }

    bool valid = true;

    if (order.price <= 0.0) valid = false;

    if (valid && order.quantity < 1) valid = false;

    if (valid && order.side != 'A' && order.side != 'B') valid = false;

    *out_is_valid = valid;

    return ERR_NONE;
}

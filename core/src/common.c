#pragma once

#include "common.h"

#include <stdio.h>
#include <stdint.h>


void cmn_check_error(ret_code_t code) {
    if(code == 0) return;
};


ret_code_t cmn_copy_order(cmn_order_t* cpy, cmn_order_t* buffer, int32_t idx) {
    cpy->client_id = buffer[idx].client_id;
    cpy->order_id = buffer[idx].order_id;
    cpy->price = buffer[idx].price;
    cpy->quantity = buffer[idx].quantity;
    cpy->side = buffer[idx].side;
    cpy->timestamp = buffer[idx].timestamp;

    return 0;
};

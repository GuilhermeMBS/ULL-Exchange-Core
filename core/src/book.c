#pragma once 

#include "common.h"
#include <stdio.h>
#include <stdint.h>

#define BUFFER_SIZE 4096

/*
Left child: 2 * i + 1
Right child: 2 * i + 2
Parent: (i - 1) / 2
*/
static Order asks[BUFFER_SIZE]; // Min Heap
static Order bids[BUFFER_SIZE]; // Max Heap


static int32_t copyOrder(Order* cpy, Order* buffer, int32_t idx) {
    cpy->client_id = buffer[idx].client_id;
    cpy->order_id = buffer[idx].order_id;
    cpy->price = buffer[idx].price;
    cpy->quantity = buffer[idx].quantity;
    cpy->side = buffer[idx].side;
    cpy->timestamp = buffer[idx].timestamp;

    return 0;
};


int32_t clearHeaps() {
    for(int i = 0; i < BUFFER_SIZE; i++) {
        bids[i].client_id = 0;
        bids[i].order_id = 0;
        bids[i].price = 0.0;
        bids[i].quantity = 0;
        bids[i].side = "";
        bids[i].timestamp = "";
    }

    for(int i = 0; i < BUFFER_SIZE; i++) {
        asks[i].client_id = 0;
        asks[i].order_id = 0;
        asks[i].price = 0.0;
        asks[i].quantity = 0;
        asks[i].side = "";
        asks[i].timestamp = "";
    }

    return 0;
}


Order getAsk() {
    Order ask_cpy;
    int32_t code = copyOrder(&ask_cpy, &asks, 0);
    checkError(code);
    return ask_cpy;
};


Order getBid() {
    Order bid_cpy;
    int32_t code = copyOrder(&bid_cpy, &bids, 0);
    checkError(code);
    return bid_cpy;
};


int32_t removeOrder() {

    return 0;
};


int32_t changeOrder() {

    return 0;
};


int32_t insertOrder() {
    return 0;
};

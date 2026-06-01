/*
    Fazer checagem de erros para tudo
    Otimização X Legibilidade
*/

#pragma once 

#include "error.h"
#include "book.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#define BUFFER_SIZE 4096


static struct obk_order_book_private_s {
    obk_order_t asks[BUFFER_SIZE]; // Min Heap
    obk_order_t bids[BUFFER_SIZE]; // Max Heap
    int32_t size_asks;
    int32_t size_bids;
};


ret_code_t cmn_copy_order(obk_order_t* cpy, obk_order_t* buffer, int32_t idx) {
    cpy->client_id = buffer[idx].client_id;
    cpy->order_id = buffer[idx].order_id;
    cpy->price = buffer[idx].price;
    cpy->quantity = buffer[idx].quantity;
    cpy->side = buffer[idx].side;
    cpy->timestamp = buffer[idx].timestamp;

    return 0;
};


ret_code_t obk_initialize_book(obk_order_book_t* st) {
    if (st == NULL) return ERR_MEM;
    
    st->size_asks = st->size_bids = 0;
    for(int i = 0; i < BUFFER_SIZE; i++) {
        st->asks[i].client_id   = st->bids[i].client_id = 0;
        st->asks[i].order_id    = st->bids[i].order_id  = 0;
        st->asks[i].price       = st->bids[i].price     = 0.0;
        st->asks[i].quantity    = st->bids[i].quantity  = 0;
        st->asks[i].side        = st->bids[i].side      = "";
        st->asks[i].timestamp   = st->bids[i].timestamp = "";
    }

    return ERR_NONE;
}


static ret_code_t obk_heapify(obk_order_t* book, int32_t i, uint32_t size) {
    const double i_price = book[i].price;
    const tm_stmp_t i_time = book[i].timestamp;
    int32_t l, r, p;
    obk_order_t tmp;
    bool stop = false;

    // Otimizar com bits?
    while(stop == false) {
        l  = 2*i + 1;
        r  = 2*i + 2;
        p  = (i - 1) / 2;

        bool c1_l = (l < size) ? (book[l].price > i_price) : false;
        bool c2_l = (l < size) ? (book[l].price == i_price) && (book[l].timestamp < i_time) : false;
        bool c1_r = (r < size) ? (book[r].price > i_price) : false;
        bool c2_r = (r < size) ? (book[r].price == i_price) && (book[r].timestamp < i_time) : false;

        if (c1_l || c2_l) {
            tmp = book[i];
            book[i] = book[l];
            book[l] = tmp;
            i = l;
        }
        else if (c1_r || c2_r) {
            tmp = book[i];
            book[i] = book[r];
            book[r] = tmp;
            i = r;
        }
        else if ((book[i].price > book[p].price) || ((book[i].price == book[p].price) && (book[i].timestamp < book[p].timestamp))) {
            tmp = book[p];
            book[p] = book[i];
            book[i] = tmp;
            i = p;
        }
        else stop = true;
    }

    return ERR_NONE;
};


ret_code_t obk_insert_order(obk_order_book_t* book, obk_order_t* cpy) {
    ret_code_t code;
    obk_order_t* side_book;
    int32_t* idx;

    if (cpy->side == 'A') {
        side_book = book->asks;
        idx = &(book->size_asks);
    }
    else if (cpy->side == 'B') {
        side_book = book->bids;
        idx = &(book->size_bids);
    }
    else return ERR_ORD;
    if (*idx == BUFFER_SIZE) return ERR_MEM;

    side_book[*idx] = *cpy;
    code = obk_heapify(side_book, *idx, (*idx)++);
    err_check_error(code);

    return ERR_NONE;
};


ret_code_t obk_remove_order(obk_order_book_t* book, char side) {
    obk_order_t* side_book;
    int32_t* book_size;
    ret_code_t code;

    if (side == 'A') {
        side_book = book->asks;
        book_size = &(book->size_asks);
    }
    else if (side == 'B') {
        side_book = book->bids;
        book_size = &(book->size_bids);
    }
    else return ERR_ORD;

    side_book[0] = side_book[(*book_size)--];
    code = obk_heapify(book, 0, *book_size);
    err_check_error(code);

    return ERR_NONE;
};


ret_code_t changeOrder(obk_order_book_t* book, uint32_t qty, char side) {
    obk_order_t* side_book;
    ret_code_t code;

    if (side == 'A') {
        side_book = book->asks;
        side_book[0].quantity = qty;
        code = obk_heapify(side_book, 0, book->size_asks);
    }
    else if (side == 'B') {
        side_book = book->bids;
        side_book[0].quantity = qty;
        code = obk_heapify(side_book, 0, book->size_bids);
    }
    else return ERR_ORD;
    err_check_error(code);

    return ERR_NONE;
};


obk_order_t obk_get_order(obk_order_book_t* book, char side) {
    obk_order_t cpy;
    ret_code_t code;
    
    if (side == 'A') code = cmn_copy_order(&cpy, book->asks, 0);
    else if (side == 'B') code = cmn_copy_order(&cpy, book->bids, 0);
    else (code = ERR_ORD);

    err_check_error(code);

    return cpy;
};

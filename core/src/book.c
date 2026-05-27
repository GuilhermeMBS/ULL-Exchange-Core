/*
    Fazer checagem de erros para tudo
    Otimização X Legibilidade
*/

#pragma once 

#include "common.h"
#include "book.h"

#include <stdio.h>
#include <stdint.h>


#define BUFFER_SIZE 4096
#define TRUE 1 // Cuidado ao fazer == TRUE (não engloba != 1 ^ !=0)
#define FALSE 0

/*
Left child: 2 * i + 1
Right child: 2 * i + 2
Parent: (i - 1) / 2
*/
static struct obk_order_book_private_s {
    cmn_order_t asks[BUFFER_SIZE]; // Min Heap
    cmn_order_t bids[BUFFER_SIZE]; // Max Heap
    int32_t size_asks;
    int32_t size_bids;
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


static ret_code_t obk_heapify(cmn_order_t* book, uint32_t i, uint32_t size) {
    uint32_t l;
    uint32_t r;
    uint32_t p;
    cmn_order_t tmp;
    uint8_t stop = FALSE;

    // Tem como escrever isto melhor?
    while(stop == FALSE) {
        l  = 2*i + 1;
        r  = 2*i + 2;
        p  = (i - 1) / 2;

        if (i == 0 || i == size) stop = TRUE;
        else if ((book[l].price > book[i].price) || ((book[l].price == book[i].price) && (book[l].timestamp < book[i].timestamp))) {
            tmp = book[i];
            book[i] = book[l];
            book[l] = tmp;
        }
        else if ((book[r].price > book[i].price) || ((book[r].price == book[i].price) && (book[r].timestamp < book[i].timestamp))) {
            tmp = book[i];
            book[i] = book[r];
            book[r] = tmp;
        }
        else if ((book[i].price > book[p].price) || ((book[i].price == book[p].price) && (book[i].timestamp < book[p].timestamp))) {
            tmp = book[p];
            book[p] = book[i];
            book[i] = tmp;
        }
        else stop = TRUE;
    }

    return ERR_NONE;
};


ret_code_t obk_insert_order(obk_order_book_t* book, cmn_order_t* cpy) {
    ret_code_t code;
    cmn_order_t* side_book;
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
    code = heapify(side_book, *idx, (*idx)++);
    cmn_check_error(code);

    return ERR_NONE;
};


ret_code_t obk_remove_order(obk_order_book_t* book, char side) {
    cmn_order_t* side_book;
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
    cmn_check_error(code);

    return ERR_NONE;
};


ret_code_t changeOrder(obk_order_book_t* book, uint32_t qty, char side) {
    cmn_order_t* side_book;
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
    cmn_check_error(code);
    
    return ERR_NONE;
};


cmn_order_t obk_get_order(obk_order_book_t* book, char side) {
    cmn_order_t cpy;
    ret_code_t code;
    
    if (side == 'A') code = cmn_copy_order(&cpy, book->asks, 0);
    else if (side == 'B') code = cmn_copy_order(&cpy, book->bids, 0);
    else (code = ERR_ORD);

    cmn_check_error(code);

    return cpy;
};

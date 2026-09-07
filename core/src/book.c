#include "retcodes.h"
#include "book.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#define BUFFER_SIZE 4096


struct obk_book_private_s {
    obk_order_t asks[BUFFER_SIZE]; // Min Heap
    obk_order_t bids[BUFFER_SIZE]; // Max Heap
    int32_t size_asks;
    int32_t size_bids;
};


typedef struct obk_book_private_s* obk_book_private_pt;

// Typedef only used in a static function, so it should not be visible to other files
typedef enum {
    CMP_ERROR   = -1,
    FIRST_IN    =  1,
    SECOND_IN   =  2,
    EQUALS      =  3
} obk_cmp_t;


ret_code_t obk_initialize_book(obk_book_pt* book) {
    if (book == NULL) return ERR_MEM;
    obk_book_private_pt new_book = malloc(sizeof(struct obk_book_private_s));
    if (new_book == NULL) return ERR_MEM;

    
    new_book->size_asks = new_book->size_bids = 0;
    for(int i = 0; i < BUFFER_SIZE; i++) {
        new_book->asks[i].client_id   = new_book->bids[i].client_id = 0;
        new_book->asks[i].order_id    = new_book->bids[i].order_id  = 0;
        new_book->asks[i].price       = new_book->bids[i].price     = 0.0;
        new_book->asks[i].quantity    = new_book->bids[i].quantity  = 0;
        new_book->asks[i].side        = new_book->bids[i].side      = '\0';
        new_book->asks[i].timestamp   = new_book->bids[i].timestamp = 0;
        new_book->asks[i].is_valid    = new_book->bids[i].is_valid  = false;
    }
    *book = new_book;

    return ERR_NONE;
}


ret_code_t obk_clear_book(obk_book_pt* book) {
    if (book == NULL || *book == NULL) return ERR_MEM;

    free(*book);
    *book = NULL;

    return ERR_NONE;
}


ret_code_t obk_copy_order(obk_order_pt cpy, obk_order_pt buffer, int32_t idx) {
    cpy->client_id = buffer[idx].client_id;
    cpy->order_id = buffer[idx].order_id;
    cpy->price = buffer[idx].price;
    cpy->quantity = buffer[idx].quantity;
    cpy->side = buffer[idx].side;
    cpy->timestamp = buffer[idx].timestamp;

    return 0;
}


/**
 * @brief This function recieves two valid orders as inputs and returns the one with the
 * highest price or the one with the smallest timestamp (in case of same price orders).
 * @attention The input order matters, because the comparison will return the answer according to it.
 * @param first_in First valid order.
 * @param second_in Second valid order.
 * @param max_heap Boolean for the type of the heap. Max = true e min = false.
 * @return Returns a obk_cmp_t that identifies if the first or the second input have the priority in 
 * the heap. For example, if its a min heap, it will return the code that identifies the input with
 * the smallest price (or smallest timestamp).
 */
static obk_cmp_t obk_cmp_order(obk_order_t first_in, obk_order_t second_in, bool max_heap) {
    double f_price = first_in.price;
    double s_price = second_in.price;

    if (f_price == s_price) {
        if (first_in.timestamp < second_in.timestamp) return FIRST_IN;
        else if (first_in.timestamp > second_in.timestamp) return SECOND_IN;
    }

    else if (f_price > s_price) {
        if (max_heap) return FIRST_IN;
        else return SECOND_IN;
    }

    else if (f_price < s_price) {
        if (max_heap) return SECOND_IN;
        else return FIRST_IN;
    }

    else return CMP_ERROR;

    return EQUALS;
}


/**
 * @brief This function heapifies any element in a heap, no matter where it is. So it goes up or down
 * according to the heap. You don't need to specify any behavior, only if it's a max or min heap.
 * @param book The heap it is located.
 * @param i The index of the element you want to heapify.
 * @param size The size of the heap.
 * @param max_heap If it's a max heap (true) or a min heap (false).
 * @return Returns a ret_code_t according to the retcodes header.
 */
static ret_code_t obk_heapify(obk_order_pt book, uint32_t i, uint32_t size, bool max_heap) {
    /**
     * cmp is a 1 byte variable that holds the information about the comparisons of i with its
     * childs and parent. The variable holds those informations as follows:
     * First bit := Left Child is a valid swap.
     * Second bit := Right Child is a valid swap.
     * Third bit := Parent is a valid swap.
     * Forth bit := Left Child and Right Child are a valid swap, but Left is the smallest/biggest.
     * Fifth bit := Left Child and Right Child are a valid swap, but Right is the smallest/biggest.
     * Others := Should be all zeroes.
     */
    uint8_t cmp;
    obk_cmp_t cmp_ret;
    obk_order_t tmp;
    uint32_t l, r, p; // left child, right child, parent

    do {
        cmp = 0x00;
        l = (2*i + 1);
        r = (2*i + 2);

        // Checks swap with left child
        if (l < size) {
            cmp_ret = obk_cmp_order(book[i], book[l], max_heap);
            if (cmp_ret == CMP_ERROR) return ERR_ORD;
            else if (cmp_ret == SECOND_IN) (cmp |= 0x01);
        }

        // Checks swap with right child
        if (r < size) {
            cmp_ret = obk_cmp_order(book[i], book[r], max_heap);
            if (cmp_ret == CMP_ERROR) return ERR_ORD;
            else if (cmp_ret == SECOND_IN) (cmp |= 0x02);
        }

        // Checks swap if both childs are valid
        if (cmp == 0x03) {
            cmp_ret = obk_cmp_order(book[l], book[r], max_heap);
            if (cmp_ret == FIRST_IN || cmp_ret == EQUALS) (cmp = 0x08);
            else if (cmp_ret == SECOND_IN) (cmp = 0x10);
            else return ERR_ORD;
        }

        // Checks swap with parent
        if (i > 0) {
            p = (i - 1) / 2;
            cmp_ret = obk_cmp_order(book[i], book[p], max_heap);
            if (cmp_ret == CMP_ERROR) return ERR_ORD;
            else if (cmp_ret == FIRST_IN) (cmp |= 0x04);
        }

        // Swaps with left child
        if (cmp & 0x09) {
            tmp = book[i];
            book[i] = book[l];
            book[l] = tmp;
            i = l;
        }

        // Swaps with right child
        else if (cmp & 0x12) {
            tmp = book[i];
            book[i] = book[r];
            book[r] = tmp;
            i = r;
        }

        // Swaps with parent
        else if (cmp & 0x04) {
            tmp = book[p];
            book[p] = book[i];
            book[i] = tmp;
            i = p;
        }
    } while(cmp);

    return ERR_NONE;
}


ret_code_t obk_insert_order(obk_book_pt book, obk_order_pt cpy) {
    ret_code_t code;
    obk_order_pt side_book;
    int32_t* idx;
    bool max_heap;

    if (cpy->side == 'A') {
        side_book = book->asks;
        idx = &(book->size_asks);
        max_heap = false;
    }
    else if (cpy->side == 'B') {
        side_book = book->bids;
        idx = &(book->size_bids);
        max_heap = true;
    }
    else return ERR_ORD;
    if (*idx >= BUFFER_SIZE) return ERR_MEM;

    side_book[*idx] = *cpy;
    code = obk_heapify(side_book, *idx, (*idx + 1), max_heap);
    (*idx)++;
    err_check_error(code);

    return ERR_NONE;
}


ret_code_t obk_remove_order(obk_book_pt book, char side) {
    obk_order_pt side_book;
    int32_t* book_size;
    ret_code_t code;
    bool max_heap;

    if (side == 'A') {
        side_book = book->asks;
        book_size = &(book->size_asks);
        max_heap = false;
    }
    else if (side == 'B') {
        side_book = book->bids;
        book_size = &(book->size_bids);
        max_heap = true;
    }
    else return ERR_ORD;
    if (*book_size <= 0) return ERR_ORD;

    (*book_size)--;
    side_book[0] = side_book[*book_size];
    code = obk_heapify(side_book, 0, *book_size, max_heap);
    err_check_error(code);

    return ERR_NONE;
}


ret_code_t obk_change_order(obk_book_pt book, uint32_t qty, char side) {
    obk_order_pt side_book;
    ret_code_t code;

    if (side == 'A') {
        side_book = book->asks;
        side_book[0].quantity = qty;
        code = obk_heapify(side_book, 0, book->size_asks, false);
    }
    else if (side == 'B') {
        side_book = book->bids;
        side_book[0].quantity = qty;
        code = obk_heapify(side_book, 0, book->size_bids, true);
    }
    else return ERR_ORD;
    err_check_error(code);

    return ERR_NONE;
}


obk_order_t obk_get_order(obk_book_pt book, char side) {
    obk_order_t cpy;
    memset(&cpy, 0, sizeof(obk_order_t));
    ret_code_t code;

    if (side == 'A') code = obk_copy_order(&cpy, book->asks, 0);
    else if (side == 'B') code = obk_copy_order(&cpy, book->bids, 0);
    else code = ERR_ORD;

    err_check_error(code);

    return cpy;
}


int32_t obk_ask_count(obk_book_pt book) {
    if (!book) return 0;
    return book->size_asks;
}

int32_t obk_bid_count(obk_book_pt book) {
    if (!book) return 0;
    return book->size_bids;
}

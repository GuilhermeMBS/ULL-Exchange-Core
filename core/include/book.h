#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "retcodes.h"


typedef uint32_t tm_stmp_t;

typedef struct {
    tm_stmp_t timestamp;
    uint32_t order_id;
    uint32_t client_id;
    uint32_t quantity;
    double price;
    char symbol[8];
    char side;
    bool is_valid;
} obk_order_t;

typedef struct obk_book_private_s* obk_book_pt; // Opaque pointer

typedef obk_order_t* obk_order_pt;


/**
 * @brief Allocates and initializes a new opaque instance of the order book.
 * @param book Double pointer to be updated with the address of the newly allocated book.
 * @return ERR_NONE on success, or ERR_MEM if the heap allocation fails.
 */
ret_code_t obk_initialize_book(obk_book_pt* book);


/**
 * @brief Safely frees the allocated heap memory and nullifies the book instance pointer.
 * @param book Double pointer to the book instance to be deallocated and cleared.
 * @return ERR_NONE on success, or ERR_MEM if the pointer is already NULL.
 */
ret_code_t obk_clear_book(obk_book_pt* book);


/**
 * @brief Explicitly copies order attributes from a source buffer index into a destination structure.
 * @param cpy Destination pointer that will receive the copied order attributes.
 * @param buffer Pointer to the source array containing active order records.
 * @param idx Index position within the source array.
 * @return 0 on success.
 */
ret_code_t obk_copy_order(obk_order_pt cpy, obk_order_pt buffer, int32_t idx);


/**
 * @brief Inserts an individual order into the correct side of the book and enforces sorting.
 * @param book Opaque handle to the trading order book instance.
 * @param cpy Pointer containing the source attributes of the incoming order.
 * @return ERR_NONE on success, ERR_ORD for an invalid side assignment, or ERR_MEM if the internal buffer is full.
 */
ret_code_t obk_insert_order(obk_book_pt book, obk_order_pt cpy);


/**
 * @brief Modifies the volume (quantity) of the highest priority active order on a given book side.
 * @param book Opaque handle to the trading order book instance.
 * @param qty New volume size to be applied onto the top-level element.
 * @param side Side descriptor identifier targeting either the sell or buy book ('A' or 'B').
 * @return ERR_NONE on success, or ERR_ORD if an invalid side identifier configuration is provided.
 */
ret_code_t obk_change_order(obk_book_pt book, uint32_t qty, char side);


/**
 * @brief Removes the highest priority order from the top of the heap structure on the specified side.
 * @param book Opaque handle to the trading order book instance.
 * @param side Side descriptor identifier targeting either the sell or buy book ('A' or 'B').
 * @return ERR_NONE on success, or ERR_ORD if the targeted book side is empty or side is invalid.
 */
ret_code_t obk_remove_order(obk_book_pt book, char side);


/**
 * @brief Returns an isolated structural copy representing the active highest priority element.
 * @param book Opaque handle to the trading order book instance.
 * @param side Side descriptor identifier targeting either the sell or buy book ('A' or 'B').
 * @return Structural copy containing the elements representing the highest priority order.
 */
obk_order_t obk_get_order(obk_book_pt book, char side);


/**
 * @brief Returns the total count of active orders residing on the ask (sell) side of the book.
 */
int32_t obk_ask_count(obk_book_pt book);


/**
 * @brief Returns the total count of active orders residing on the bid (buy) side of the book.
 */
int32_t obk_bid_count(obk_book_pt book);

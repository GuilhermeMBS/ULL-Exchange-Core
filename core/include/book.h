/*
Teste de Ordenação: Inserir 3 ordens de compra (Preços: 10.00, 12.00, 11.00). 
O getBestBid deve retornar a de 12.00.

Teste de Prioridade Temporal: Inserir duas ordens de 10.00. 
A que foi inserida primeiro deve sair primeiro.

Teste de Remoção: Remover uma ordem do meio da fila e garantir que os ponteiros 
next não se perderam (não deixar a lista "quebrada").
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "errorlib.h"


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

typedef struct obk_book_private_s* obk_book_pt; // Opaque Pointer

typedef obk_order_t* obk_order_pt;


/**
 * @brief Allocates and initializes a new opaque order book instance.
 * @param book Double pointer to be updated with the new book address.
 * @return ERR_NONE on success, ERR_MEM if allocation fails.
 */
ret_code_t obk_initialize_book(obk_book_pt* book);

/**
 * @brief Safely releases allocated heap memory and nullifies the pointer.
 * @param book Double pointer to the order book instance to clear.
 * @return ERR_NONE on success, ERR_MEM if pointer is already NULL.
 */
ret_code_t obk_clear_book(obk_book_pt* book);

/**
 * @brief Explicitly copies field data from a buffer index to a target structure.
 * @param cpy Target pointer receiving the order attributes.
 * @param buffer Source array pointer containing the active records.
 * @param idx Array indexing coordinate.
 * @return 0 on success.
 */
ret_code_t obk_copy_order(obk_order_pt cpy, obk_order_pt buffer, int32_t idx);

/**
 * @brief Inserts an order into the appropriate execution book side and sorts it.
 * @param book Opaque handle to the trading book.
 * @param cpy Pointer to the incoming order data.
 * @return ERR_NONE on success, ERR_ORD on invalid side, ERR_MEM on buffer saturation.
 */
ret_code_t obk_insert_order(obk_book_pt book, obk_order_pt cpy);

/**
 * @brief Modifies the volume (quantity) of the top-priority order on a given side.
 * @param book Opaque handle to the trading book.
 * @param qty New volume to override at the top order index.
 * @param side Target book side identifier ('A' or 'B').
 * @return ERR_NONE on success, ERR_ORD on invalid side configuration.
 */
ret_code_t obk_change_order(obk_book_pt book, uint32_t qty, char side);

/**
 * @brief Removes the highest-priority order from the top of the designated side heap.
 * @param book Opaque handle to the trading book.
 * @param side Target book side identifier ('A' or 'B').
 * @return ERR_NONE on success, ERR_ORD if book is empty or invalid side.
 */
ret_code_t obk_remove_order(obk_book_pt book, char side);

/**
 * @brief Returns an isolated, decoupled copy of the highest-priority order.
 * @param book Opaque handle to the trading book.
 * @param side Target book side identifier ('A' or 'B').
 * @return A structural copy instance representing the prioritized element.
 */
obk_order_t obk_get_order(obk_book_pt book, char side); // Side = 'A' / 'B'

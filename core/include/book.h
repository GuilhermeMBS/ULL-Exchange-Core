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


ret_code_t obk_initialize_book(obk_book_pt* book);
ret_code_t obk_clear_book(obk_book_pt* book);

ret_code_t obk_copy_order(obk_order_pt cpy, obk_order_pt buffer, int32_t idx);

ret_code_t obk_insert_order(obk_book_pt book, obk_order_pt cpy);
ret_code_t obk_change_order(obk_book_pt book, uint32_t qty, char side);
ret_code_t obk_remove_order(obk_book_pt book, char side);

obk_order_t obk_get_order(obk_book_pt book, char side); // Side = 'A' / 'B'

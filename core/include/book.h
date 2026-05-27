/*
Teste de Ordenação: Inserir 3 ordens de compra (Preços: 10.00, 12.00, 11.00). 
O getBestBid deve retornar a de 12.00.

Teste de Prioridade Temporal: Inserir duas ordens de 10.00. 
A que foi inserida primeiro deve sair primeiro.

Teste de Remoção: Remover uma ordem do meio da fila e garantir que os ponteiros 
next não se perderam (não deixar a lista "quebrada").
*/

#include "common.h"


typedef struct obk_order_book_private_s obk_order_book_t;


ret_code_t obk_initialize_book(obk_order_book_t* st);

ret_code_t obk_insert_order(obk_order_book_t* book, cmn_order_t* cpy);
ret_code_t obk_change_order(int32_t id, int32_t qty, char side);
ret_code_t obk_remove_order(obk_order_book_t* book, char side);

cmn_order_t obk_get_order(obk_order_book_t* book, char side); // Side = 'A' / 'B'

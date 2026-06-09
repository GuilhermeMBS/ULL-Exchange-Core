#pragma once

#include "error.h"
#include "book.h"


typedef struct {
    uint32_t timestamp;          // Momento exato do match
    int32_t trade_id;            // ID único do negócio realizado
    int32_t buy_order_id;        // ID da ordem de compra original
    int32_t sell_order_id;       // ID da ordem de venda original
    int32_t buy_client_id;       // ID do comprador
    int32_t sell_client_id;      // ID do vendedor
    double price;                // Preço final da execução (pode ser diferente do pedido)
    int32_t quantity;            // Quantidade que foi trocada
} mtc_transaction_t;


/* 
Retorno: 0 (sem Match → foi para o Book),  1 (Match Total), 2 (Match Parcial).
Erro: -1 (ordem inválida recebida).
*/
int32_t mtc_make_trade(obk_order_t* incoming);


/* 
Retorno: 0 (processado).
Erro: -1 (falha na comunicação com o Book).
*/
int32_t mtc_make_bid(obk_order_t* order);
int32_t mtc_make_sell(obk_order_t* order);

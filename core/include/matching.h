#pragma once

#include "errorlib.h"
#include "book.h"


typedef struct {
    uint32_t timestamp;         // Momento exato do casamento
    int32_t trade_id;           // Identificador único da negociação
    int32_t buy_order_id;       // ID da ordem de compra original
    int32_t sell_order_id;      // ID da ordem de venda original
    int32_t buy_client_id;      // ID do cliente comprador
    int32_t sell_client_id;     // ID do cliente vendedor
    double price;               // Preço final de execução
    int32_t quantity;           // Quantidade negociada
} mtc_transaction_t;


/**
 * @brief Tenta casar a ordem recebida com o livro atual.
 * @param incoming Ponteiro para a ordem recebida.
 * @return 0 (sem casamento — enfileirada no livro), 1 (casamento total), 2 (casamento parcial).
 *         ERR_ORD em caso de ordem inválida.
 */
ret_code_t mtc_make_trade(obk_order_t* incoming);


/**
 * @brief Processa uma ordem de compra no livro.
 * @param order Ponteiro para a ordem a ser processada.
 * @return 0 em caso de sucesso, ERR_ORD em falha de comunicação com o livro.
 */
ret_code_t mtc_make_bid(obk_order_t* order);

/**
 * @brief Processa uma ordem de venda no livro.
 * @param order Ponteiro para a ordem a ser processada.
 * @return 0 em caso de sucesso, ERR_ORD em falha de comunicação com o livro.
 */
ret_code_t mtc_make_sell(obk_order_t* order);


/** @brief Introspecção — reinicia o estado e inspeciona o livro interno (suporte a testes). */
void        mtc_reset(void);
ret_code_t  mtc_get_ask_count(void);
ret_code_t  mtc_get_bid_count(void);
obk_order_t mtc_get_best_ask(void);
obk_order_t mtc_get_best_bid(void);

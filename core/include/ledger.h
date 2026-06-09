#pragma once

#include "errorlib.h"
#include "matching.h"

/**
 * @file ledger.h
 * @brief Módulo de registro binário de transações.
 */

/**
 * @brief Agrupa um array de transações e sua contagem.
 */
typedef struct {
    mtc_transaction_t* data;
    int32_t count;
} ldg_buffer_t;


/**
 * @brief Inicializa o arquivo de ledger binário no caminho fornecido.
 * @param bin_path Caminho para o arquivo .bin a ser criado.
 * @return 0 em caso de sucesso, -1 em erro de permissão.
 */
ret_code_t ldg_init_ledger(const char* bin_path);

/**
 * @brief Adiciona uma única transação ao ledger.
 * @param t Ponteiro para a transação a ser registrada.
 * @return 0 em caso de sucesso, -3 em falha de escrita.
 */
ret_code_t ldg_register_trade(mtc_transaction_t* t);

/**
 * @brief Escreve todas as transações do buffer no ledger.
 * @param transactions Ponteiro para o ldg_buffer_t com as transações.
 * @return 0 em caso de sucesso, -3 em entrada inválida ou falha de escrita.
 */
ret_code_t ldg_register_all(ldg_buffer_t* transactions);

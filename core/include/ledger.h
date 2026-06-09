#pragma once

#include "errorlib.h"
#include "matching.h"

/*
  @file ledger.h
  @brief Módulo de registro de transações em arquivo binário.
 */

 /*
 Agrupa um array de transações e sua quantidade.
 */
typedef struct {
    mtc_transaction_t* data;  /* Array de transações */
    uint32_t count;                /* Número de transações no array */
} Buffer;


/*
  Inicializa o arquivo binário do ledger no caminho especificado.
 
  @param bin_path  Caminho do arquivo .bin a ser criado
  @return          0 se sucesso, -1 se erro de permissão
 */
int32_t ldg_init_ledger(const char* bin_path);

/*
  Registra uma única transação no ledger.
 @param t  Ponteiro para a transação a ser gravada
 @return   0 se gravado com sucesso, -1 se falha de escrita
 */
// int32_t ldg_register_trade(mtc_transaction_t* t);

/*
  Grava todas as transações do buffer no arquivo "ledger.bin".
 
  @param transactions  Ponteiro para o Buffer com as transações
  @return              0 se sucesso, -3 se entrada inválida ou falha de escrita
 */
ret_code_t ldg_register_trades(Buffer* transactions);

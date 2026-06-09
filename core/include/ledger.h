#ifndef LEDGER_H
#define LEDGER_H

#include "errorlib.h"
#include "matching.h"

/*
  ledger.h - Módulo de registro de transações em arquivo binário.
*/

/*
  Inicializa o arquivo binário do ledger no caminho especificado.
  @param bin_path  Caminho do arquivo .bin a ser criado
  @return          0 se sucesso, -1 se erro de permissão
*/
int32_t ldg_init_ledger(const char* bin_path);

/*
  Registra uma única transação no ledger.
  @param t  Ponteiro para a transação a ser gravada
  @return   0 se gravado com sucesso, -3 se falha de escrita
*/
int32_t ldg_register_trade(mtc_transaction_t* t);

#endif
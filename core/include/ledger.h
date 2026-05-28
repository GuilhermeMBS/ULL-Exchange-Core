
#include "common.h"

/*
Retorno: 0.
Erro: -1 (permissão negada para criar o arquivo).
*/
int32_t ldg_init_ledger(const char* bin_path);

/*
Retorno: 0 (gravado com sucesso).
Erro: -1 (falha de escrita no disco / espaço cheio).
*/
int32_t ldg_register_trade(cmn_transaction_t* t);

#ifndef LEDGER_H
#define LEDGER_H

#include <common.h>  // Transaction definido aqui

typedef struct {
    Transaction* data;
    int count;
} Buffer;

int registerTrades(Buffer* transactions);

#endif


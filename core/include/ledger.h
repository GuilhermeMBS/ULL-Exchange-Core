#include "error.h"
#include "matching.h"

/*
Retorno: 0.
Erro: -1 (permissão negada para criar o arquivo).
*/
int32_t ldg_init_ledger(const char* bin_path);

/*
Retorno: 0 (gravado com sucesso).
Erro: -1 (falha de escrita no disco / espaço cheio).
*/
int32_t ldg_register_trade(mtc_transaction_t* t);

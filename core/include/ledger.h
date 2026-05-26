#include "common.h"

/*
Retorno: 0.
Erro: -1 (permissão negada para criar o arquivo).
*/
int initLedger(const char* bin_path);

/*
Retorno: 0 (gravado com sucesso).
Erro: -1 (falha de escrita no disco / espaço cheio).
*/
int registerTrade(Transaction* t);

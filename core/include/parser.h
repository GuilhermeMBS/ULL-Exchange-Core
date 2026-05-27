#include "common.h"


/*
Retorno: Ponteiro para o início do array de Order na RAM.
Erro: NULL se o arquivo não existir ou se houver falha de malloc.
*/
cmn_order_t* prs_create_orders(const char* csv_path, int32_t* total_count);

/*
Retorno: 0 (sucesso).
Erro: -1 se o ponteiro já for nulo.
*/
int32_t prs_free_buffer(cmn_order_t* buffer);

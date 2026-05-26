#include "common.h"


/*
Retorno: Ponteiro para o início do array de Order na RAM.
Erro: NULL se o arquivo não existir ou se houver falha de malloc.
*/
Order* createOrders(const char* csv_path, int* total_count);

/*
Retorno: 0 (sucesso).
Erro: -1 se o ponteiro já for nulo.
*/
int freeBuffer(Order* buffer);

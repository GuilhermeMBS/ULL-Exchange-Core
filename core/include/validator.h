#include "common.h"


/*
Retorno: Nenhum (altera o order_id para -1 nas ordens inválidas).
Erro Interno: se o preço ou quantidade ≤ 0 ou side ≠ ‘C’ ou ‘V’.
*/
void vld_validate_order(cmn_order_t* buffer, int32_t count);

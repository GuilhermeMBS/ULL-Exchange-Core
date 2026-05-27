#include "common.h"


/* 
Retorno: 0 (sem Match → foi para o Book),  1 (Match Total), 2 (Match Parcial).
Erro: -1 (ordem inválida recebida).
*/
int32_t mtc_make_trade(cmn_order_t* incoming);


/* 
Retorno: 0 (processado).
Erro: -1 (falha na comunicação com o Book).
*/
int32_t mtc_make_bid(cmn_order_t* order);
int32_t mtc_make_sell(cmn_order_t* order);

#include "common.h"


/* 
Retorno: 0 (sem Match → foi para o Book),  1 (Match Total), 2 (Match Parcial).
Erro: -1 (ordem inválida recebida).
*/
int makeTrade(Order* incoming);


/* 
Retorno: 0 (processado).
Erro: -1 (falha na comunicação com o Book).
*/
int makeBid(Order* order);
int makeSell(Order* order);

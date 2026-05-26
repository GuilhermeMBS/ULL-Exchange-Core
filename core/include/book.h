<<<<<<< Updated upstream
=======
/*
Teste de Ordenação: Inserir 3 ordens de compra (Preços: 10.00, 12.00, 11.00). 
O getBestBid deve retornar a de 12.00.

Teste de Prioridade Temporal: Inserir duas ordens de 10.00. 
A que foi inserida primeiro deve sair primeiro.

Teste de Remoção: Remover uma ordem do meio da fila e garantir que os ponteiros 
next não se perderam (não deixar a lista "quebrada").
*/

#include "common.h"


/*
Retorno: Ponteiro para a melhor ordem do topo.
Erro: NULL se a fila estiver vazia (importante para o Matching parar de procurar match).
*/
int32_t clearHeaps();

Order getBid();

Order getAsk();

int32_t insertOrder(Order* cpy);

int32_t changeOrder(int32_t id, int32_t qty, char side);

int32_t removeOrder(int32_t id);
>>>>>>> Stashed changes

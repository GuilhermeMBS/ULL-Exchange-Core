// Estruturas e variáveis compartilhadas por todos os arquivos C

#include <stdio.h>
#include <stdint.h>


#define ERR_NONE 0
#define ERR_ORD -1
#define ERR_MEM -3


typedef int8_t ret_code_t;


typedef struct {
    uint32_t timestamp;
    uint32_t order_id;
    uint32_t client_id;
    uint32_t quantity;
    double price;
    char symbol[8];
    char side;
    char is_valid;
} cmn_order_t;


typedef struct {
    uint32_t timestamp;          // Momento exato do match
    int32_t trade_id;            // ID único do negócio realizado
    int32_t buy_order_id;        // ID da ordem de compra original
    int32_t sell_order_id;       // ID da ordem de venda original
    int32_t buy_client_id;       // ID do comprador
    int32_t sell_client_id;      // ID do vendedor
    double price;                // Preço final da execução (pode ser diferente do pedido)
    int32_t quantity;            // Quantidade que foi trocada
} cmn_transaction_t;


void cmn_check_error(ret_code_t code);

ret_code_t cmn_copy_order(cmn_order_t* cpy, cmn_order_t* buffer, int32_t idx);

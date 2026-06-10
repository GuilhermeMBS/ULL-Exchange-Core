#pragma once

#include "retcodes.h"
#include "book.h"
#include "parserlib.h"


typedef struct {
    uint32_t timestamp;         // Momento exato do casamento
    int32_t trade_id;           // Identificador único da negociação
    int32_t buy_order_id;       // ID da ordem de compra original
    int32_t sell_order_id;      // ID da ordem de venda original
    int32_t buy_client_id;      // ID do cliente comprador
    int32_t sell_client_id;     // ID do cliente vendedor
    double price;               // Preço final de execução
    int32_t quantity;           // Quantidade negociada
} mtc_transaction_t;


typedef struct mtc_instance_private_s* mtc_handle_pt; // Opaque pointer


/**
 * @brief Allocates and initializes a new matching engine instance.
 * @param handle Double pointer targeted to receive the address of the allocated engine instance.
 * @return ERR_NONE on success, or ERR_MEM if heap allocation fails.
 */
ret_code_t mtc_create_engine(mtc_handle_pt* handle);


/**
 * @brief Processes all orders from the parser sequentially by using its opaque handle interface.
 * @param handle Opaque handle targeting the specific matching engine instance.
 * @param prs_handle Opaque handle pointing to the populated parser memory container.
 * @param total_orders The total number of processed entries residing inside the parser buffer.
 * @param out_total_trades Pointer updated with the final count of executed transactions.
 * @return ERR_NONE on success, or ERR_ORD if an engine error or invalid status is encountered.
 */
ret_code_t mtc_process_matching(mtc_handle_pt handle, prs_handle_pt prs_handle, int32_t total_orders, int32_t* out_total_trades);


/**
 * @brief Safely fetches an executed trade record from the engine storage array by its index.
 * @param handle Opaque handle targeting the matching engine instance.
 * @param idx The targeted transaction array index position.
 * @param out_trade Destination pointer where the transaction data will be copied.
 * @return ERR_NONE on success, or ERR_ORD if the index is out of bounds or parameters are invalid.
 */
ret_code_t mtc_get_trade_by_index(mtc_handle_pt handle, int32_t idx, mtc_transaction_t* out_trade);


/**
 * @brief Frees the matching engine instance, its inner order book, and transaction records.
 * @param handle Double pointer targeting the instance handle to clear and nullify.
 * @return ERR_NONE on success, or ERR_MEM if the instance pointer is already invalid.
 */
ret_code_t mtc_clear_engine(mtc_handle_pt* handle);


/** @brief Introspecção — support helper tools allowing introspection into the internal book instance state. */
ret_code_t  mtc_get_ask_count(mtc_handle_pt handle);
ret_code_t  mtc_get_bid_count(mtc_handle_pt handle);
obk_order_t mtc_get_best_ask(mtc_handle_pt handle);
obk_order_t mtc_get_best_bid(mtc_handle_pt handle);

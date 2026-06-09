#pragma once

#include "errorlib.h"
#include "book.h"


typedef struct {
    uint32_t timestamp;         // Exact moment of the match
    int32_t trade_id;           // Unique trade identifier
    int32_t buy_order_id;       // ID of the original buy order
    int32_t sell_order_id;      // ID of the original sell order
    int32_t buy_client_id;      // Buyer client ID
    int32_t sell_client_id;     // Seller client ID
    double price;               // Final execution price
    int32_t quantity;           // Traded quantity
} mtc_transaction_t;


/*
Return: 0 (no match — queued in book), 1 (full match), 2 (partial match).
Error: -1 (invalid order received).
*/
ret_code_t mtc_make_trade(obk_order_t* incoming);


/*
Return: 0 (processed).
Error: -1 (book communication failure).
*/
ret_code_t mtc_make_bid(obk_order_t* order);
ret_code_t mtc_make_sell(obk_order_t* order);


/* Introspection — reset state and inspect internal book (test support) */
void        mtc_reset(void);
ret_code_t  mtc_get_ask_count(void);
ret_code_t  mtc_get_bid_count(void);
obk_order_t mtc_get_best_ask(void);
obk_order_t mtc_get_best_bid(void);

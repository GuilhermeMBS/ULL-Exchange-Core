#include <time.h>

#include "matching.h"
#include "ledger.h"

static obk_book_pt s_book    = NULL;
static int32_t     s_trade_id = 0;


ret_code_t mtc_make_bid(obk_order_t* order) {
    if (!order) return ERR_ORD;

    if (obk_ask_count(s_book) == 0)
        return (int32_t)obk_insert_order(s_book, order);

    obk_order_t best_ask = obk_get_order(s_book, 'A');

    if (order->price < best_ask.price)
        return (int32_t)obk_insert_order(s_book, order);

    uint32_t qty = order->quantity < best_ask.quantity
                   ? order->quantity : best_ask.quantity;

    mtc_transaction_t t = {
        .timestamp      = (uint32_t)time(NULL),
        .trade_id       = ++s_trade_id,
        .buy_order_id   = (int32_t)order->order_id,
        .sell_order_id  = (int32_t)best_ask.order_id,
        .buy_client_id  = (int32_t)order->client_id,
        .sell_client_id = (int32_t)best_ask.client_id,
        .price          = best_ask.price,
        .quantity       = (int32_t)qty,
    };
    ldg_register_trade(&t);

    if (order->quantity == best_ask.quantity) {
        obk_remove_order(s_book, 'A');
        return 1;
    }
    if (order->quantity > best_ask.quantity) {
        obk_remove_order(s_book, 'A');
        order->quantity -= qty;
        int32_t r = mtc_make_bid(order);
        return (r == 1) ? 1 : 2;
    }
    obk_change_order(s_book, best_ask.quantity - qty, 'A');
    return 2;
}


ret_code_t mtc_make_sell(obk_order_t* order) {
    if (!order) return ERR_ORD;

    if (obk_bid_count(s_book) == 0)
        return (int32_t)obk_insert_order(s_book, order);

    obk_order_t best_bid = obk_get_order(s_book, 'B');

    if (order->price > best_bid.price)
        return (int32_t)obk_insert_order(s_book, order);

    uint32_t qty = order->quantity < best_bid.quantity
                   ? order->quantity : best_bid.quantity;

    mtc_transaction_t t = {
        .timestamp      = (uint32_t)time(NULL),
        .trade_id       = ++s_trade_id,
        .buy_order_id   = (int32_t)best_bid.order_id,
        .sell_order_id  = (int32_t)order->order_id,
        .buy_client_id  = (int32_t)best_bid.client_id,
        .sell_client_id = (int32_t)order->client_id,
        .price          = best_bid.price,
        .quantity       = (int32_t)qty,
    };
    ldg_register_trade(&t);

    if (order->quantity == best_bid.quantity) {
        obk_remove_order(s_book, 'B');
        return 1;
    }
    if (order->quantity > best_bid.quantity) {
        obk_remove_order(s_book, 'B');
        order->quantity -= qty;
        int32_t r = mtc_make_sell(order);
        return (r == 1) ? 1 : 2;
    }
    obk_change_order(s_book, best_bid.quantity - qty, 'B');
    return 2;
}


ret_code_t mtc_make_trade(obk_order_t* incoming) {
    if (!incoming) return ERR_ORD;
    if (!s_book) {
        if (obk_initialize_book(&s_book) != ERR_NONE) return ERR_MEM;
    }
    if (incoming->side == 'B') return mtc_make_bid(incoming);
    if (incoming->side == 'A') return mtc_make_sell(incoming);
    return ERR_ORD;
}


void mtc_reset(void) {
    if (s_book) obk_clear_book(&s_book);
    obk_initialize_book(&s_book);
    s_trade_id = 0;
}

ret_code_t mtc_get_ask_count(void) { return obk_ask_count(s_book); }
ret_code_t mtc_get_bid_count(void) { return obk_bid_count(s_book); }

obk_order_t mtc_get_best_ask(void) { return obk_get_order(s_book, 'A'); }
obk_order_t mtc_get_best_bid(void) { return obk_get_order(s_book, 'B'); }

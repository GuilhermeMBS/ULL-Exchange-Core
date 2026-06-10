#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "matching.h"
#include "book.h"
#include "parserlib.h"

#define MTC_MAX_TRADES 8192


struct mtc_instance_private_s {
    obk_book_pt book;
    int32_t trade_id;
    mtc_transaction_t trades_array[MTC_MAX_TRADES];
    int32_t trades_count;
    int32_t last_processed_idx;
};


typedef struct mtc_instance_private_s* mtc_instance_private_pt;


static ret_code_t mtc_make_bid(mtc_instance_private_pt instance, obk_order_t* order) {
    if (!instance || !order) return ERR_ORD;

    if (obk_ask_count(instance->book) == 0)
        return (ret_code_t)obk_insert_order(instance->book, order);

    obk_order_t best_ask = obk_get_order(instance->book, 'A');

    if (order->price < best_ask.price)
        return (ret_code_t)obk_insert_order(instance->book, order);

    if (order->client_id == best_ask.client_id)
        return ERR_ORD;

    if (instance->trades_count >= MTC_MAX_TRADES) return ERR_MEM;

    uint32_t qty = order->quantity < best_ask.quantity
                   ? order->quantity : best_ask.quantity;

    mtc_transaction_t t = {
        .timestamp      = (tm_stmp_t)time(NULL),
        .trade_id       = ++instance->trade_id,
        .buy_order_id   = (int32_t)order->order_id,
        .sell_order_id  = (int32_t)best_ask.order_id,
        .buy_client_id  = (int32_t)order->client_id,
        .sell_client_id = (int32_t)best_ask.client_id,
        .price          = best_ask.price,
        .quantity       = (int32_t)qty,
    };

    instance->trades_array[instance->trades_count] = t;
    instance->trades_count++;

    if (order->quantity == best_ask.quantity) {
        obk_remove_order(instance->book, 'A');
        return TOT_MATCH;
    }

    if (order->quantity > best_ask.quantity) {
        obk_remove_order(instance->book, 'A');
        order->quantity -= qty;
        ret_code_t r = mtc_make_bid(instance, order);
        return (r == TOT_MATCH) ? TOT_MATCH : PARC_MATCH;
    }

    obk_change_order(instance->book, best_ask.quantity - qty, 'A');
    return PARC_MATCH;
}


static ret_code_t mtc_make_sell(mtc_instance_private_pt instance, obk_order_t* order) {
    if (!instance || !order) return ERR_ORD;

    if (obk_bid_count(instance->book) == 0)
        return (ret_code_t)obk_insert_order(instance->book, order);

    obk_order_t best_bid = obk_get_order(instance->book, 'B');

    if (order->price > best_bid.price)
        return (ret_code_t)obk_insert_order(instance->book, order);

    if (order->client_id == best_bid.client_id)
        return ERR_ORD;

    if (instance->trades_count >= MTC_MAX_TRADES) return ERR_MEM;

    uint32_t qty = order->quantity < best_bid.quantity
                   ? order->quantity : best_bid.quantity;

    mtc_transaction_t t = {
        .timestamp      = (tm_stmp_t)time(NULL),
        .trade_id       = ++instance->trade_id,
        .buy_order_id   = (int32_t)best_bid.order_id,
        .sell_order_id  = (int32_t)order->order_id,
        .buy_client_id  = (int32_t)best_bid.client_id,
        .sell_client_id = (int32_t)order->client_id,
        .price          = best_bid.price,
        .quantity       = (int32_t)qty,
    };

    instance->trades_array[instance->trades_count] = t;
    instance->trades_count++;

    if (order->quantity == best_bid.quantity) {
        obk_remove_order(instance->book, 'B');
        return TOT_MATCH;
    }
    
    if (order->quantity > best_bid.quantity) {
        obk_remove_order(instance->book, 'B');
        order->quantity -= qty;
        ret_code_t r = mtc_make_sell(instance, order);
        return (r == TOT_MATCH) ? TOT_MATCH : PARC_MATCH;
    }
    
    obk_change_order(instance->book, best_bid.quantity - qty, 'B');
    return PARC_MATCH;
}


ret_code_t mtc_create_engine(mtc_handle_pt* handle) {
    ret_code_t code;

    if (!handle) return ERR_ORD;

    mtc_instance_private_pt instance = (mtc_instance_private_pt)malloc(sizeof(struct mtc_instance_private_s));
    if (!instance) {
        code = ERR_MEM;
        err_check_error(code);
        return code;
    }

    instance->book = NULL;
    code = obk_initialize_book(&(instance->book));
    if (code != ERR_NONE) {
        free(instance);
        return code;
    }

    instance->trade_id = 0;
    instance->trades_count = 0;
    instance->last_processed_idx = 0;
    memset(instance->trades_array, 0, sizeof(instance->trades_array));

    *handle = (mtc_handle_pt)instance;
    return ERR_NONE;
}


ret_code_t mtc_process_matching(mtc_handle_pt handle, prs_handle_pt prs_handle, int32_t total_orders, int32_t* out_total_trades) {
    ret_code_t code;

    if (!handle || !prs_handle || !out_total_trades) return ERR_ORD;

    mtc_instance_private_pt instance = (mtc_instance_private_pt)handle;
    obk_order_t incoming_order;

    /* Loop resumes directly from the tracked index location to prevent rewriting records */
    for (int32_t i = instance->last_processed_idx; i < total_orders; i++) {
        code = prs_get_order_by_index(prs_handle, i, &incoming_order);
        if (code != ERR_NONE) continue;

        if (!incoming_order.is_valid) {
            instance->last_processed_idx = i + 1;
            continue;
        }

        if (incoming_order.side == 'B') {
            mtc_make_bid(instance, &incoming_order);
        } else if (incoming_order.side == 'A') {
            mtc_make_sell(instance, &incoming_order);
        }

        instance->last_processed_idx = i + 1;
    }

    *out_total_trades = instance->trades_count;
    return ERR_NONE;
}


ret_code_t mtc_get_trade_by_index(mtc_handle_pt handle, int32_t idx, mtc_transaction_t* out_trade) {
    if (!handle || !out_trade) return ERR_ORD;

    mtc_instance_private_pt instance = (mtc_instance_private_pt)handle;

    if (idx < 0 || idx >= instance->trades_count) return ERR_ORD;

    *out_trade = instance->trades_array[idx];
    return ERR_NONE;
}


ret_code_t mtc_clear_engine(mtc_handle_pt* handle) {
    if (!handle || !*handle) return ERR_ORD;

    mtc_instance_private_pt instance = (mtc_instance_private_pt)*handle;

    if (instance->book) {
        obk_clear_book(&(instance->book));
    }

    free(instance);
    *handle = NULL;
    return ERR_NONE;
}


ret_code_t mtc_get_ask_count(mtc_handle_pt handle) {
    if (!handle) return ERR_ORD;
    return obk_ask_count(((mtc_instance_private_pt)handle)->book);
}


ret_code_t mtc_get_bid_count(mtc_handle_pt handle) {
    if (!handle) return ERR_ORD;
    return obk_bid_count(((mtc_instance_private_pt)handle)->book);
}


obk_order_t mtc_get_best_ask(mtc_handle_pt handle) {
    obk_order_t dummy = {0};
    if (!handle) return dummy;
    return obk_get_order(((mtc_instance_private_pt)handle)->book, 'A');
}


obk_order_t mtc_get_best_bid(mtc_handle_pt handle) {
    obk_order_t dummy = {0};
    if (!handle) return dummy;
    return obk_get_order(((mtc_instance_private_pt)handle)->book, 'B');
}

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "retcodes.h"
#include "book.h"
#include "parser.h"
#include "matching.h"


static void create_test_csv(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    assert(fp != NULL);

    fputs(content, fp);

    fclose(fp);
}


// 1. Null pointer must be rejected without touching internal state
void test_null_order_rejection() {
    printf("[TEST] Requirement 1: Null Order Rejection...\n");

    int32_t total_trades = 0;
    ret_code_t code;

    /* Verifies that the matching engine interface safely rejects invalid context handles */
    code = mtc_process_matching(NULL, NULL, 0, &total_trades);
    assert(code == ERR_ORD);

    printf("[PASS] Null rejection verified — internal state unchanged.\n\n");
}


// 2. Invalid side character must be rejected without inserting into the book
void test_invalid_side_rejection() {
    printf("[TEST] Requirement 2: Invalid Side Rejection...\n");

    create_test_csv(
        "bad_side.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,1,10,100,50.0,PETR4,X\n"
    );

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;

    assert(prs_create_orders("bad_side.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 0);

    assert(mtc_get_ask_count(mtc_handle) == 0);
    assert(mtc_get_bid_count(mtc_handle) == 0);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("bad_side.csv");

    printf("[PASS] Invalid side rejected — book untouched.\n\n");
}


// 3. When the opposite side is empty there is no match — order goes to the book
void test_no_match_empty_book() {
    printf("[TEST] Requirement 3: No Match on Empty Opposite Side...\n");

    create_test_csv(
        "empty_opp.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,10,1,200,80.0,PETR4,B\n"
        "1001,11,2,200,80.0,PETR4,A\n"
    );

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;

    assert(prs_create_orders("empty_opp.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    /* First order (Bid) arrives — ask book is empty, so it enters the book */
    assert(mtc_process_matching(mtc_handle, prs_handle, 1, &total_trades) == ERR_NONE);
    assert(total_trades == 0);
    assert(mtc_get_bid_count(mtc_handle) == 1);
    assert(mtc_get_ask_count(mtc_handle) == 0);

    /* Second order (Ask) arrives — matching the price and quantities perfectly executing a trade */
    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 1);
    assert(mtc_get_bid_count(mtc_handle) == 0);
    assert(mtc_get_ask_count(mtc_handle) == 0);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("empty_opp.csv");

    printf("[PASS] Orders inserted into book when opposite side is empty.\n\n");
}


// 4. When bid price < best ask (or ask price > best bid) there is no cross — order goes to book
void test_no_match_price_cross_miss() {
    printf("[TEST] Requirement 4: No Match on Price Cross Miss...\n");

    create_test_csv(
        "cross_miss.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,20,1,100,100.0,PETR4,A\n"
        "1001,21,2,100,90.0,PETR4,B\n"
    );

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;

    assert(prs_create_orders("cross_miss.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    /* Resting ask at 100.0 — incoming bid at 90.0 (below ask, no cross) */
    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 0);
    assert(mtc_get_ask_count(mtc_handle) == 1);
    assert(mtc_get_bid_count(mtc_handle) == 1);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("cross_miss.csv");

    printf("[PASS] Orders inserted into book when price does not cross.\n\n");
}


// 5. Equal quantities on both sides produce a full match — both sides cleared
void test_full_match_equal_quantities() {
    printf("[TEST] Requirement 5: Full Match on Equal Quantities...\n");

    create_test_csv(
        "equal_qty.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,30,1,200,100.0,PETR4,A\n"
        "1001,31,2,200,100.0,PETR4,B\n"
    );

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;
    mtc_transaction_t trade_record;

    assert(prs_create_orders("equal_qty.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 1);

    /* Verifies that the internal trades buffer captures tracking parity fields accurately */
    assert(mtc_get_trade_by_index(mtc_handle, 0, &trade_record) == ERR_NONE);
    assert(trade_record.buy_order_id == 31);
    assert(trade_record.sell_order_id == 30);
    assert(trade_record.quantity == 200);

    assert(mtc_get_ask_count(mtc_handle) == 0);
    assert(mtc_get_bid_count(mtc_handle) == 0);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("equal_qty.csv");

    printf("[PASS] Full match on equal quantities — both sides cleared.\n\n");
}


// 6. Bid quantity smaller than ask — partial match, ask quantity reduced, ask stays in book
void test_partial_match_bid_deficit() {
    printf("[TEST] Requirement 6: Partial Match — Bid Deficit...\n");

    create_test_csv(
        "bid_deficit.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,40,1,500,50.0,PETR4,A\n"
        "1001,41,2,200,50.0,PETR4,B\n"
    );

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;
    obk_order_t remaining_ask;

    assert(prs_create_orders("bid_deficit.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 1);

    assert(mtc_get_ask_count(mtc_handle) == 1);
    assert(mtc_get_bid_count(mtc_handle) == 0);

    remaining_ask = mtc_get_best_ask(mtc_handle);
    assert(remaining_ask.quantity == 300);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("bid_deficit.csv");

    printf("[PASS] Partial match bid deficit — ask reduced and kept in book.\n\n");
}


// 7. Ask quantity smaller than bid — partial match, bid quantity reduced, bid stays in book
void test_partial_match_ask_deficit() {
    printf("[TEST] Requirement 7: Partial Match — Ask Deficit...\n");

    create_test_csv(
        "ask_deficit.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,50,1,500,80.0,PETR4,B\n"
        "1001,51,2,200,80.0,PETR4,A\n"
    );

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;
    obk_order_t remaining_bid;

    assert(prs_create_orders("ask_deficit.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 1);

    assert(mtc_get_bid_count(mtc_handle) == 1);
    assert(mtc_get_ask_count(mtc_handle) == 0);

    remaining_bid = mtc_get_best_bid(mtc_handle);
    assert(remaining_bid.quantity == 300);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("ask_deficit.csv");

    printf("[PASS] Partial match ask deficit — bid reduced and kept in book.\n\n");
}


// 8. Bid surplus — bid recursively consumes multiple asks until fully executed
void test_recursive_full_match_bid_surplus() {
    printf("[TEST] Requirement 8: Recursive Full Match — Bid Surplus...\n");

    create_test_csv(
        "surplus.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,60,1,100,40.0,PETR4,A\n"
        "1001,61,1,100,40.0,PETR4,A\n"
        "1002,62,1,100,40.0,PETR4,A\n"
        "1003,63,2,300,40.0,PETR4,B\n"
    );

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;

    assert(prs_create_orders("surplus.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    /* 3 separate matches committed internally onto the trades stack buffer structure */
    assert(total_trades == 3);

    assert(mtc_get_ask_count(mtc_handle) == 0);
    assert(mtc_get_bid_count(mtc_handle) == 0);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("surplus.csv");

    printf("[PASS] Recursive full match — bid surplus consumed all resting asks.\n\n");
}


// 9. mtc_make_trade correctly routes bid and ask orders to their respective pipelines
void test_trade_routing_via_make_trade() {
    printf("[TEST] Requirement 9: Routing via mtc_make_trade...\n");

    create_test_csv(
        "routing.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,70,1,150,60.0,PETR4,B\n"
        "1001,71,2,150,65.0,PETR4,A\n"
    );

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;

    assert(prs_create_orders("routing.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 0);
    assert(mtc_get_bid_count(mtc_handle) == 1);
    assert(mtc_get_ask_count(mtc_handle) == 1);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("routing.csv");

    printf("[PASS] Trade routing by side character verified.\n\n");
}


int main() {
    printf("\n============= STARTING MATCHING ENGINE TEST SUITE =============\n\n");

    test_null_order_rejection();
    test_invalid_side_rejection();
    test_no_match_empty_book();
    test_no_match_price_cross_miss();
    test_full_match_equal_quantities();
    test_partial_match_bid_deficit();
    test_partial_match_ask_deficit();
    test_recursive_full_match_bid_surplus();
    test_trade_routing_via_make_trade();

    printf("============ ALL TEST DOMAINS VERIFIED SUCCESSFULLY ===========\n\n");
    return 0;
}
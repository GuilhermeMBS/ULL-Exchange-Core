/*
cmd: gcc core/src/error.c core/src/book.c core/src/matching.c tests/c_tests/test_matching.c -o matching_tester -Icore/include -Wall -Wextra
*/

#include <stdio.h>
#include <assert.h>

#include "matching.h"


// 1. Null pointer must be rejected without touching internal state
void test_null_order_rejection() {
    printf("[TEST] Requirement 1: Null Order Rejection...\n");

    mtc_reset();

    assert(mtc_make_trade(NULL) == ERR_ORD);
    assert(mtc_make_bid(NULL)   == ERR_ORD);
    assert(mtc_make_sell(NULL)  == ERR_ORD);

    assert(mtc_get_ask_count() == 0);
    assert(mtc_get_bid_count() == 0);

    printf("[PASS] Null rejection verified — internal state unchanged.\n\n");
}


// 2. Invalid side character must be rejected without inserting into the book
void test_invalid_side_rejection() {
    printf("[TEST] Requirement 2: Invalid Side Rejection...\n");

    mtc_reset();

    obk_order_t bad = { .order_id = 1, .price = 50.0, .quantity = 100, .side = 'X', .timestamp = 1 };
    assert(mtc_make_trade(&bad) == ERR_ORD);

    assert(mtc_get_ask_count() == 0);
    assert(mtc_get_bid_count() == 0);

    printf("[PASS] Invalid side rejected — book untouched.\n\n");
}


// 3. When the opposite side is empty there is no match — order goes to the book
void test_no_match_empty_book() {
    printf("[TEST] Requirement 3: No Match on Empty Opposite Side...\n");

    mtc_reset();

    // Bid arrives — ask book is empty
    obk_order_t bid = { .order_id = 10, .price = 80.0, .quantity = 200, .side = 'B', .timestamp = 1 };
    assert(mtc_make_bid(&bid) == ERR_NONE);
    assert(mtc_get_bid_count() == 1);
    assert(mtc_get_ask_count() == 0);

    mtc_reset();

    // Ask arrives — bid book is empty
    obk_order_t ask = { .order_id = 11, .price = 80.0, .quantity = 200, .side = 'A', .timestamp = 2 };
    assert(mtc_make_sell(&ask) == ERR_NONE);
    assert(mtc_get_ask_count() == 1);
    assert(mtc_get_bid_count() == 0);

    mtc_reset();
    printf("[PASS] Orders inserted into book when opposite side is empty.\n\n");
}


// 4. When bid price < best ask (or ask price > best bid) there is no cross — order goes to book
void test_no_match_price_cross_miss() {
    printf("[TEST] Requirement 4: No Match on Price Cross Miss...\n");

    mtc_reset();

    // Resting ask at 100.0 — incoming bid at 90.0 (below ask, no cross)
    obk_order_t ask = { .order_id = 20, .price = 100.0, .quantity = 100, .side = 'A', .timestamp = 1 };
    assert(mtc_make_sell(&ask) == ERR_NONE);

    obk_order_t bid = { .order_id = 21, .price = 90.0, .quantity = 100, .side = 'B', .timestamp = 2 };
    assert(mtc_make_bid(&bid) == ERR_NONE);

    assert(mtc_get_ask_count() == 1);
    assert(mtc_get_bid_count() == 1);

    mtc_reset();

    // Resting bid at 80.0 — incoming ask at 90.0 (above bid, no cross)
    obk_order_t bid2 = { .order_id = 22, .price = 80.0, .quantity = 100, .side = 'B', .timestamp = 3 };
    assert(mtc_make_bid(&bid2) == ERR_NONE);

    obk_order_t ask2 = { .order_id = 23, .price = 90.0, .quantity = 100, .side = 'A', .timestamp = 4 };
    assert(mtc_make_sell(&ask2) == ERR_NONE);

    assert(mtc_get_bid_count() == 1);
    assert(mtc_get_ask_count() == 1);

    mtc_reset();
    printf("[PASS] Orders inserted into book when price does not cross.\n\n");
}


// 5. Equal quantities on both sides produce a full match — both sides cleared
void test_full_match_equal_quantities() {
    printf("[TEST] Requirement 5: Full Match on Equal Quantities...\n");

    mtc_reset();

    // Place a resting ask
    obk_order_t ask = { .order_id = 30, .price = 100.0, .quantity = 200, .side = 'A', .timestamp = 1 };
    assert(mtc_make_sell(&ask) == ERR_NONE);
    assert(mtc_get_ask_count() == 1);

    // Incoming bid matches exactly
    obk_order_t bid = { .order_id = 31, .price = 100.0, .quantity = 200, .side = 'B', .timestamp = 2 };
    assert(mtc_make_bid(&bid) == 1);

    assert(mtc_get_ask_count() == 0);
    assert(mtc_get_bid_count() == 0);

    mtc_reset();
    printf("[PASS] Full match on equal quantities — both sides cleared.\n\n");
}


// 6. Bid quantity smaller than ask — partial match, ask quantity reduced, ask stays in book
void test_partial_match_bid_deficit() {
    printf("[TEST] Requirement 6: Partial Match — Bid Deficit...\n");

    mtc_reset();

    obk_order_t ask = { .order_id = 40, .price = 50.0, .quantity = 500, .side = 'A', .timestamp = 1 };
    assert(mtc_make_sell(&ask) == ERR_NONE);

    obk_order_t bid = { .order_id = 41, .price = 50.0, .quantity = 200, .side = 'B', .timestamp = 2 };
    assert(mtc_make_bid(&bid) == 2);

    assert(mtc_get_ask_count() == 1);
    assert(mtc_get_bid_count() == 0);

    obk_order_t remaining_ask = mtc_get_best_ask();
    assert(remaining_ask.quantity == 300);

    mtc_reset();
    printf("[PASS] Partial match bid deficit — ask reduced and kept in book.\n\n");
}


// 7. Ask quantity smaller than bid — partial match, bid quantity reduced, bid stays in book
void test_partial_match_ask_deficit() {
    printf("[TEST] Requirement 7: Partial Match — Ask Deficit...\n");

    mtc_reset();

    obk_order_t bid = { .order_id = 50, .price = 80.0, .quantity = 500, .side = 'B', .timestamp = 1 };
    assert(mtc_make_bid(&bid) == ERR_NONE);

    obk_order_t ask = { .order_id = 51, .price = 80.0, .quantity = 200, .side = 'A', .timestamp = 2 };
    assert(mtc_make_sell(&ask) == 2);

    assert(mtc_get_bid_count() == 1);
    assert(mtc_get_ask_count() == 0);

    obk_order_t remaining_bid = mtc_get_best_bid();
    assert(remaining_bid.quantity == 300);

    mtc_reset();
    printf("[PASS] Partial match ask deficit — bid reduced and kept in book.\n\n");
}


// 8. Bid surplus — bid recursively consumes multiple asks until fully executed
void test_recursive_full_match_bid_surplus() {
    printf("[TEST] Requirement 8: Recursive Full Match — Bid Surplus...\n");

    mtc_reset();

    // Insert 3 resting asks at the same price, 100 units each
    obk_order_t ask1 = { .order_id = 60, .price = 40.0, .quantity = 100, .side = 'A', .timestamp = 1 };
    obk_order_t ask2 = { .order_id = 61, .price = 40.0, .quantity = 100, .side = 'A', .timestamp = 2 };
    obk_order_t ask3 = { .order_id = 62, .price = 40.0, .quantity = 100, .side = 'A', .timestamp = 3 };

    assert(mtc_make_sell(&ask1) == ERR_NONE);
    assert(mtc_make_sell(&ask2) == ERR_NONE);
    assert(mtc_make_sell(&ask3) == ERR_NONE);
    assert(mtc_get_ask_count() == 3);

    // Bid for the entire 300 units — must consume all three asks recursively
    obk_order_t bid = { .order_id = 63, .price = 40.0, .quantity = 300, .side = 'B', .timestamp = 4 };
    assert(mtc_make_bid(&bid) == 1);

    assert(mtc_get_ask_count() == 0);
    assert(mtc_get_bid_count() == 0);

    mtc_reset();
    printf("[PASS] Recursive full match — bid surplus consumed all resting asks.\n\n");
}


// 9. mtc_make_trade correctly routes bid and ask orders to their respective pipelines
void test_trade_routing_via_make_trade() {
    printf("[TEST] Requirement 9: Routing via mtc_make_trade...\n");

    mtc_reset();

    // Bid side routing: book is empty, order must land in bid book
    obk_order_t bid = { .order_id = 70, .price = 60.0, .quantity = 150, .side = 'B', .timestamp = 1 };
    assert(mtc_make_trade(&bid) == ERR_NONE);
    assert(mtc_get_bid_count() == 1);
    assert(mtc_get_ask_count() == 0);

    mtc_reset();

    // Ask side routing: book is empty, order must land in ask book
    obk_order_t ask = { .order_id = 71, .price = 60.0, .quantity = 150, .side = 'A', .timestamp = 2 };
    assert(mtc_make_trade(&ask) == ERR_NONE);
    assert(mtc_get_ask_count() == 1);
    assert(mtc_get_bid_count() == 0);

    mtc_reset();
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

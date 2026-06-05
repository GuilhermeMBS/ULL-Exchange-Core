/*
cmd: gcc core/src/errorlib.c core/src/book.c tests/c_tests/test_book.c -o book_tester -Icore/include -Wall -Wextra
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "book.h"
#include "errorlib.h"


#define BUFFER_SIZE 4096


// Forward declaration of internal function to allow unit testing of copying logic
ret_code_t obk_copy_order(obk_order_pt cpy, obk_order_pt buffer, int32_t idx);

// 1. Validate book initialization and instance separation
void test_initialization_and_isolation() {
    printf("[TEST] Requirement 1: Initialization and Instance Isolation...\n");

    obk_book_pt execution_book_a = NULL;
    obk_book_pt execution_book_b = NULL;

    // Verify successful instantiation
    assert(obk_initialize_book(&execution_book_a) == ERR_NONE);
    assert(obk_initialize_book(&execution_book_b) == ERR_NONE);
    assert(execution_book_a != NULL && execution_book_b != NULL);

    // Verify data segregation between instances
    obk_order_t order_a = { .order_id = 101, .price = 50.0, .side = 'B', .timestamp = 1 };
    assert(obk_insert_order(execution_book_a, &order_a) == ERR_NONE);

    obk_order_t top_a = obk_get_order(execution_book_a, 'B');
    assert(top_a.order_id == 101);

    // Clean up memory
    assert(obk_clear_book(&execution_book_a) == ERR_NONE);
    assert(obk_clear_book(&execution_book_b) == ERR_NONE);
    
    printf("[PASS] Initialization and isolation verified successfully.\n\n");
}

// 2. Validate clean-up logic and safe pointer nullification
void test_clear_book_lifecycle() {
    printf("[TEST] Requirement 2: Book Clear Lifecycle...\n");

    obk_book_pt temporary_book = NULL;
    assert(obk_initialize_book(&temporary_book) == ERR_NONE);
    assert(temporary_book != NULL);

    // Verify pointer modification after free operation
    assert(obk_clear_book(&temporary_book) == ERR_NONE);
    assert(temporary_book == NULL);

    printf("[PASS] Clear book lifecycle validated.\n\n");
}

// 3. Validate accurate memory copying of structural fields
void test_order_field_copying() {
    printf("[TEST] Requirement 3: Order Field Copying Integrity...\n");

    obk_order_t source_buffer[1];
    source_buffer[0].client_id = 77;
    source_buffer[0].order_id = 5005;
    source_buffer[0].price = 125.75;
    source_buffer[0].quantity = 1000;
    source_buffer[0].side = 'B';
    source_buffer[0].timestamp = 123456;

    obk_order_t destination_order;
    memset(&destination_order, 0, sizeof(obk_order_t));

    // Invoke copy mechanism
    obk_copy_order(&destination_order, source_buffer, 0);

    // Assert absolute parity across data layouts
    assert(destination_order.client_id == source_buffer[0].client_id);
    assert(destination_order.order_id == source_buffer[0].order_id);
    assert(destination_order.price == source_buffer[0].price);
    assert(destination_order.quantity == source_buffer[0].quantity);
    assert(destination_order.side == source_buffer[0].side);
    assert(destination_order.timestamp == source_buffer[0].timestamp);

    printf("[PASS] Structural field copying verified.\n\n");
}

// 4. Validate dual heap invariants (Min/Max Heapify) and Time-Priority tie-breaking
void test_heap_invariants_and_time_priority() {
    printf("[TEST] Requirement 4: Heapify Invariants and Time-Priority Sorting...\n");

    obk_book_pt trading_book = NULL;
    assert(obk_initialize_book(&trading_book) == ERR_NONE);

    // --- TEST CASE A: BIDS (Max Heap - Highest Price Wins) ---
    obk_order_t bid_low  = { .order_id = 1, .price = 10.00, .side = 'B', .timestamp = 100 };
    obk_order_t bid_high = { .order_id = 2, .price = 20.00, .side = 'B', .timestamp = 101 };
    obk_order_t bid_mid  = { .order_id = 3, .price = 15.00, .side = 'B', .timestamp = 102 };

    assert(obk_insert_order(trading_book, &bid_low) == ERR_NONE);
    assert(obk_insert_order(trading_book, &bid_high) == ERR_NONE);
    assert(obk_insert_order(trading_book, &bid_mid) == ERR_NONE);

    obk_order_t best_bid = obk_get_order(trading_book, 'B');
    assert(best_bid.price == 20.00 && best_bid.order_id == 2);

    // --- TEST CASE B: ASKS (Min Heap - Lowest Price Wins) ---
    obk_order_t ask_high = { .order_id = 4, .price = 90.00, .side = 'A', .timestamp = 200 };
    obk_order_t ask_low  = { .order_id = 5, .price = 80.00, .side = 'A', .timestamp = 201 };
    obk_order_t ask_mid  = { .order_id = 6, .price = 85.00, .side = 'A', .timestamp = 202 };

    assert(obk_insert_order(trading_book, &ask_high) == ERR_NONE);
    assert(obk_insert_order(trading_book, &ask_low) == ERR_NONE);
    assert(obk_insert_order(trading_book, &ask_mid) == ERR_NONE);

    obk_order_t best_ask = obk_get_order(trading_book, 'A');
    assert(best_ask.price == 80.00 && best_ask.order_id == 5);

    // --- TEST CASE C: TIME PRIORITY (First arrival breaks ties) ---
    // Inserting an identical price to best_bid (20.00) but arriving FIRST (timestamp 90 < 101)
    obk_order_t bid_tie = { .order_id = 7, .price = 20.00, .side = 'B', .timestamp = 90 };
    assert(obk_insert_order(trading_book, &bid_tie) == ERR_NONE);

    // Top order must be order_id 7 now
    best_bid = obk_get_order(trading_book, 'B');
    assert(best_bid.order_id == 7);

    assert(obk_clear_book(&trading_book) == ERR_NONE);
    printf("[PASS] Dual heap constraints and temporal priority validated.\n\n");
}

// 5. Validate orderly extraction from the structure and read idempotency
void test_removal_and_get_order_behavior() {
    printf("[TEST] Requirement 5: Top Removal Re-sorting and Reader Idempotency...\n");

    obk_book_pt trading_book = NULL;
    assert(obk_initialize_book(&trading_book) == ERR_NONE);

    obk_order_t b1 = { .order_id = 1, .price = 30.00, .side = 'B', .timestamp = 10 };
    obk_order_t b2 = { .order_id = 2, .price = 45.00, .side = 'B', .timestamp = 11 };
    obk_order_t b3 = { .order_id = 3, .price = 35.00, .side = 'B', .timestamp = 12 };

    assert(obk_insert_order(trading_book, &b1) == ERR_NONE);
    assert(obk_insert_order(trading_book, &b2) == ERR_NONE);
    assert(obk_insert_order(trading_book, &b3) == ERR_NONE);

    // Read verification is idempotent and returns a decoupled structural copy
    obk_order_t fetch_1 = obk_get_order(trading_book, 'B');
    obk_order_t fetch_2 = obk_get_order(trading_book, 'B');
    assert(fetch_1.order_id == fetch_2.order_id && fetch_1.price == fetch_2.price);

    // Modify copy to prove encapsulation
    fetch_1.price = 0.00;
    obk_order_t fallback_check = obk_get_order(trading_book, 'B');
    assert(fallback_check.price == 45.00);

    // Removing top element forces standard down-heap operations
    assert(obk_remove_order(trading_book, 'B') == ERR_NONE);
    obk_order_t post_removal_top = obk_get_order(trading_book, 'B');
    assert(post_removal_top.price == 35.00 && post_removal_top.order_id == 3);

    assert(obk_clear_book(&trading_book) == ERR_NONE);
    printf("[PASS] Extraction and read decoupling mechanics verified.\n\n");
}

// 6. Validate in-place value amendment and heap stabilization
void test_change_order_modification() {
    printf("[TEST] Requirement 6: Top-Order In-Place Volume Modifications...\n");

    obk_book_pt trading_book = NULL;
    assert(obk_initialize_book(&trading_book) == ERR_NONE);

    obk_order_t base_order = { .order_id = 99, .price = 150.00, .side = 'A', .quantity = 100, .timestamp = 5 };
    assert(obk_insert_order(trading_book, &base_order) == ERR_NONE);

    // Modify volume parameters at the target leaf node index
    assert(obk_change_order(trading_book, 25, 'A') == ERR_NONE);

    obk_order_t validated_order = obk_get_order(trading_book, 'A');
    assert(validated_order.quantity == 25);

    assert(obk_clear_book(&trading_book) == ERR_NONE);
    printf("[PASS] Volume modification updates committed successfully.\n\n");
}

// 7. Validate bounded matrix arrays under max threshold limits
void test_buffer_boundary_saturation() {
    printf("[TEST] Requirement 7: Array Saturation and ERR_MEM Prevention...\n");

    obk_book_pt boundary_book = NULL;
    assert(obk_initialize_book(&boundary_book) == ERR_NONE);

    // Fill the book completely to its operational maximum (4096 orders)
    for (int i = 0; i < BUFFER_SIZE; i++) {
        obk_order_t internal_order = {
            .order_id = i,
            .price = 10.0 + i,
            .quantity = 50,
            .side = 'B',
            .timestamp = i,
            .is_valid = true
        };
        // Every single one of these 4096 orders must insert successfully (ERR_NONE)
        assert(obk_insert_order(boundary_book, &internal_order) == ERR_NONE);
    }

    // Attempt to insert the 4097th order (Overflow condition)
    obk_order_t overflow_order = {
        .order_id = 9999,
        .price = 50.0,
        .quantity = 10,
        .side = 'B',
        .timestamp = 20000,
        .is_valid = true
    };

    // It should safely return ERR_MEM back to the caller.
    ret_code_t result = obk_insert_order(boundary_book, &overflow_order);
    assert(result == ERR_MEM);

    assert(obk_clear_book(&boundary_book) == ERR_NONE);
    printf("[PASS] Matrix saturation limits and safe error returns verified.\n\n");
}

int main() {
    printf("\n============== STARTING SIMULATION ENGINE SUITE ===============\n\n");
    
    test_initialization_and_isolation();
    test_clear_book_lifecycle();
    test_order_field_copying();
    test_heap_invariants_and_time_priority();
    test_removal_and_get_order_behavior();
    test_change_order_modification();
    test_buffer_boundary_saturation();

    printf("============ ALL TEST DOMAINS VERIFIED SUCCESSFULLY ===========\n\n");
    return 0;
}
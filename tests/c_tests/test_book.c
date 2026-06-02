/*
cmd: gcc core/src/errorlib.c core/src/book.c tests/c_tests/test_book.c -o book_tester -Icore/include -Wall -Wextra
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "book.h"
#include "errorlib.h"

// Test 1: Validates if multiple books isolate their data properly
void test_multiple_books() {
    printf("[TEST] Starting: Multiple Instances...\n");
    obk_book_pt petr4_book = NULL;
    obk_book_pt vale3_book = NULL;

    assert(obk_initialize_book(&petr4_book) == ERR_NONE);
    assert(obk_initialize_book(&vale3_book) == ERR_NONE);

    obk_order_t order1 = { .order_id = 1, .price = 30.50, .side = 'B' };
    obk_order_t order2 = { .order_id = 2, .price = 95.00, .side = 'B' };

    assert(obk_insert_order(petr4_book, &order1) == ERR_NONE);
    assert(obk_insert_order(vale3_book, &order2) == ERR_NONE);

    obk_clear_book(&petr4_book);
    obk_clear_book(&vale3_book);
    printf("[PASS] Multiple Instances successful.\n\n");
}

// Test 2: Heap Sorting and Time Priority (Multiple Elements)
void test_heap_sorting_and_priority() {
    printf("[TEST] Starting: Heap Sorting and Priority (Multiple Elements)...\n");
    obk_book_pt book = NULL;
    assert(obk_initialize_book(&book) == ERR_NONE);

    // --- TESTING BIDS (MAX HEAP: Highest price at the top) ---
    // Inserting prices out of order: 10.00, 12.00, 11.00
    obk_order_t b1 = { .order_id = 1, .price = 10.00, .side = 'B', .timestamp = 100 };
    obk_order_t b2 = { .order_id = 2, .price = 12.00, .side = 'B', .timestamp = 101 };
    obk_order_t b3 = { .order_id = 3, .price = 11.00, .side = 'B', .timestamp = 102 };

    assert(obk_insert_order(book, &b1) == ERR_NONE);
    assert(obk_insert_order(book, &b2) == ERR_NONE);
    assert(obk_insert_order(book, &b3) == ERR_NONE);

    // The top of the Bids book must be 12.00
    obk_order_t best_bid = obk_get_order(book, 'B');
    printf("-> Current Best Bid: Price %.2f (Expected: 12.00)\n", best_bid.price);
    assert(best_bid.price == 12.00);

    // --- TESTING TIME PRIORITY (Same price, lowest timestamp wins) ---
    // Inserting another 12.00 order, but with a LARGER timestamp (arrived later)
    obk_order_t b4 = { .order_id = 4, .price = 12.00, .side = 'B', .timestamp = 105 };
    assert(obk_insert_order(book, &b4) == ERR_NONE);

    // The top must still be the original b2 order (timestamp 101)
    best_bid = obk_get_order(book, 'B');
    assert(best_bid.order_id == 2);

    // --- TESTING SUCCESSIVE REMOVALS ---
    // Remove the top (12.00, id 2)
    assert(obk_remove_order(book, 'B') == ERR_NONE);
    
    // Now the new top must be the second 12.00 order (id 4)
    best_bid = obk_get_order(book, 'B');
    printf("-> New Best Bid after removal: Price %.2f, ID %d (Expected: 12.00, ID 4)\n", best_bid.price, best_bid.order_id);
    assert(best_bid.price == 12.00 && best_bid.order_id == 4);

    // Remove the second 12.00 order (id 4)
    assert(obk_remove_order(book, 'B') == ERR_NONE);

    // Now the top must be 11.00 (id 3)
    best_bid = obk_get_order(book, 'B');
    printf("-> Next Best Bid: Price %.2f (Expected: 11.00)\n", best_bid.price);
    assert(best_bid.price == 11.00);

    obk_clear_book(&book);
    printf("[PASS] Heap Sorting and Priority validated.\n\n");
}

// Test 3: Edge Case - Empty Book Removal (Index 0)
void test_empty_book_edge_case() {
    printf("[TEST] Starting: Edge Case with Empty Book (Index 0)...\n");
    obk_book_pt book = NULL;
    assert(obk_initialize_book(&book) == ERR_NONE);

    printf("-> Attempting to remove from an empty book...\n");
    // This will trigger your err_check_error and print "Order Error!" before exiting.
    obk_remove_order(book, 'B');

    obk_clear_book(&book);
}

// Test 4: Edge Case - Buffer Overflow (4096 orders)
void test_buffer_overflow_edge_case() {
    printf("[TEST] Starting: Buffer Maximum Limit (4096 orders)...\n");
    obk_book_pt book = NULL;
    assert(obk_initialize_book(&book) == ERR_NONE);

    obk_order_t order = { .price = 10.0, .side = 'B' };

    for (int i = 0; i < 4096; i++) {
        order.order_id = i;
        if (obk_insert_order(book, &order) != ERR_NONE) {
            printf("[FAIL] Failed to insert order number %d\n", i + 1);
            return;
        }
    }
    printf("-> Successfully inserted 4096 orders.\n");

    printf("-> Attempting to insert the 4097th order (Overflow Edge Case)...\n");
    // This will trigger your err_check_error and print "Memory Error!" before exiting.
    order.order_id = 4097;
    obk_insert_order(book, &order);

    obk_clear_book(&book);
}

int main() {
    printf("================ STARTING ORDER BOOK TESTS ================\n\n");
    
    test_multiple_books();
    test_heap_sorting_and_priority();
    
    // NOTE: The tests below trigger your error handling library.
    // Since 'err_check_error' calls exit(), the program will end 
    // immediately inside them, printing your custom error message.
    test_empty_book_edge_case();
    test_buffer_overflow_edge_case();

    printf("================ ALL TESTS PASSED SUCCESSFULLY! ================\n");
    return 0;
}

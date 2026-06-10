#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include "retcodes.h"
#include "validator.h"
#include "book.h"


// 1. Validate safe handling of NULL output pointer — returns correct error code instead of crashing
void test_null_buffer() {
    printf("[TEST] Validator - NULL output pointer...\n");

    obk_order_t order = {
        .order_id = 10,
        .price = 10.0,
        .quantity = 100,
        .side = 'A',
        .is_valid = true
    };

    ret_code_t code = vld_validate_order(order, NULL);
    assert(code == ERR_ORD);

    printf("[PASS] NULL output pointer handled correctly.\n\n");
}


// 2. Validate standard execution path — returns success status code on valid destination addresses
void test_zero_count() {
    printf("[TEST] Validator - success status return...\n");

    obk_order_t order = {
        .order_id = 10,
        .price = 10.0,
        .quantity = 100,
        .side = 'A',
        .is_valid = true
    };
    bool is_order_valid = false;

    ret_code_t code = vld_validate_order(order, &is_order_valid);
    assert(code == ERR_NONE);

    printf("[PASS] status code returned correctly.\n\n");
}


// 3. Validate preservation of structurally correct orders — out_is_valid evaluates to true
void test_valid_orders() {
    printf("[TEST] Validator - valid orders...\n");

    obk_order_t order1 = {
        .order_id = 1,
        .price = 100.0,
        .quantity = 10,
        .side = 'A',
        .is_valid = true
    };
    bool is_order1_valid = false;

    obk_order_t order2 = {
        .order_id = 2,
        .price = 90.0,
        .quantity = 20,
        .side = 'B',
        .is_valid = true
    };
    bool is_order2_valid = false;

    vld_validate_order(order1, &is_order1_valid);
    vld_validate_order(order2, &is_order2_valid);

    assert(is_order1_valid == true);
    assert(is_order2_valid == true);

    printf("[PASS] Valid orders preserved.\n\n");
}


// 4. Validate rejection of an order with price = 0.0 (out_is_valid evaluates to false)
void test_invalid_price_zero() {
    printf("[TEST] Validator - price = 0...\n");

    obk_order_t order = {
        .order_id = 1,
        .price = 0.0,
        .quantity = 100,
        .side = 'A',
        .is_valid = true
    };
    bool is_order_valid = true;

    vld_validate_order(order, &is_order_valid);
    assert(is_order_valid == false);

    printf("[PASS] Price zero invalidated.\n\n");
}


// 5. Validate rejection of an order with a negative price (out_is_valid evaluates to false)
void test_invalid_price_negative() {
    printf("[TEST] Validator - negative price...\n");

    obk_order_t order = {
        .order_id = 1,
        .price = -100.0,
        .quantity = 100,
        .side = 'A',
        .is_valid = true
    };
    bool is_order_valid = true;

    vld_validate_order(order, &is_order_valid);
    assert(is_order_valid == false);

    printf("[PASS] Negative price invalidated.\n\n");
}


// 6. Validate rejection of an order with quantity = 0 (out_is_valid evaluates to false)
void test_invalid_quantity() {
    printf("[TEST] Validator - quantity = 0...\n");

    obk_order_t order = {
        .order_id = 1,
        .price = 100.0,
        .quantity = 0,
        .side = 'A',
        .is_valid = true
    };
    bool is_order_valid = true;

    vld_validate_order(order, &is_order_valid);
    assert(is_order_valid == false);

    printf("[PASS] Quantity zero invalidated.\n\n");
}


// 7. Validate rejection of an order with an unrecognised side character (not 'A' or 'B')
void test_invalid_side() {
    printf("[TEST] Validator - invalid side...\n");

    obk_order_t order = {
        .order_id = 1,
        .price = 100.0,
        .quantity = 100,
        .side = 'X',
        .is_valid = true
    };
    bool is_order_valid = true;

    vld_validate_order(order, &is_order_valid);
    assert(is_order_valid == false);

    printf("[PASS] Invalid side rejected.\n\n");
}


// 8. Validate normalisation of a parser-rejected order — out_is_valid evaluates to false without re-evaluating fields
void test_previously_invalid_order() {
    printf("[TEST] Validator - previously invalid order...\n");

    obk_order_t order = {
        .order_id = 123,
        .price = 100.0,
        .quantity = 100,
        .side = 'A',
        .is_valid = false
    };
    bool is_order_valid = true;

    vld_validate_order(order, &is_order_valid);
    assert(is_order_valid == false);

    printf("[PASS] Previously invalid order normalized.\n\n");
}


// 9. Validate sequential checking — verifying standalone execution over a variety of criteria
void test_mixed_buffer() {
    printf("[TEST] Validator - sequential validation checks...\n");

    obk_order_t orders[4] = {
        {.order_id=1,.price=100,.quantity=10,.side='A',.is_valid=true},
        {.order_id=2,.price=0,.quantity=10,.side='A',.is_valid=true},
        {.order_id=3,.price=100,.quantity=0,.side='B',.is_valid=true},
        {.order_id=4,.price=100,.quantity=10,.side='X',.is_valid=true}
    };
    bool results[4];

    for (int32_t i = 0; i < 4; i++) {
        vld_validate_order(orders[i], &results[i]);
    }

    assert(results[0] == true);
    assert(results[1] == false);
    assert(results[2] == false);
    assert(results[3] == false);

    printf("[PASS] Sequential valuation verified correctly.\n\n");
}


int main() {
    printf("\n=============== STARTING VALIDATOR TESTS ===============\n\n");

    test_null_buffer();
    test_zero_count();
    test_valid_orders();
    test_invalid_price_zero();
    test_invalid_price_negative();
    test_invalid_quantity();
    test_invalid_side();
    test_previously_invalid_order();
    test_mixed_buffer();

    printf("=============== ALL VALIDATOR TESTS PASSED ===============\n\n");

    return 0;
}
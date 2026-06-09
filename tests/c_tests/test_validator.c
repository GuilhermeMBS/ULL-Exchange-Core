#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include "validator.h"
#include "book.h"

// 1. Validate safe handling of NULL buffer — no crash or undefined behaviour
void test_null_buffer() {
    printf("[TEST] Validator - NULL buffer...\n");

    vld_validate_order(NULL, 10);

    printf("[PASS] NULL buffer handled correctly.\n\n");
}

// 2. Validate no-op behaviour when count = 0 — valid order left untouched
void test_zero_count() {
    printf("[TEST] Validator - count = 0...\n");

    obk_order_t order = {
        .order_id = 10,
        .price = 10.0,
        .quantity = 100,
        .side = 'A',
        .is_valid = true
    };

    vld_validate_order(&order, 0);

    assert(order.order_id == 10);
    assert(order.is_valid == true);

    printf("[PASS] count = 0 handled correctly.\n\n");
}

// 3. Validate preservation of structurally correct orders — is_valid remains true
void test_valid_orders() {
    printf("[TEST] Validator - valid orders...\n");

    obk_order_t orders[2] = {
        {
            .order_id = 1,
            .price = 100.0,
            .quantity = 10,
            .side = 'A',
            .is_valid = true
        },
        {
            .order_id = 2,
            .price = 90.0,
            .quantity = 20,
            .side = 'B',
            .is_valid = true
        }
    };

    vld_validate_order(orders, 2);

    assert(orders[0].is_valid == true);
    assert(orders[1].is_valid == true);

    printf("[PASS] Valid orders preserved.\n\n");
}

// 4. Validate rejection of an order with price = 0.0 (is_valid = false, order_id = -1)
void test_invalid_price_zero() {
    printf("[TEST] Validator - price = 0...\n");

    obk_order_t order = {
        .order_id = 1,
        .price = 0.0,
        .quantity = 100,
        .side = 'A',
        .is_valid = true
    };

    vld_validate_order(&order, 1);

    assert(order.is_valid == false);
    assert(order.order_id == (uint32_t)-1);

    printf("[PASS] Price zero invalidated.\n\n");
}

// 5. Validate rejection of an order with a negative price (is_valid = false, order_id = -1)
void test_invalid_price_negative() {
    printf("[TEST] Validator - negative price...\n");

    obk_order_t order = {
        .order_id = 1,
        .price = -100.0,
        .quantity = 100,
        .side = 'A',
        .is_valid = true
    };

    vld_validate_order(&order, 1);

    assert(order.is_valid == false);
    assert(order.order_id == (uint32_t)-1);

    printf("[PASS] Negative price invalidated.\n\n");
}

// 6. Validate rejection of an order with quantity = 0 (is_valid = false, order_id = -1)
void test_invalid_quantity() {
    printf("[TEST] Validator - quantity = 0...\n");

    obk_order_t order = {
        .order_id = 1,
        .price = 100.0,
        .quantity = 0,
        .side = 'A',
        .is_valid = true
    };

    vld_validate_order(&order, 1);

    assert(order.is_valid == false);
    assert(order.order_id == (uint32_t)-1);

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

    vld_validate_order(&order, 1);

    assert(order.is_valid == false);
    assert(order.order_id == (uint32_t)-1);

    printf("[PASS] Invalid side rejected.\n\n");
}

// 8. Validate normalisation of a parser-rejected order — order_id set to -1 without re-evaluating fields
void test_previously_invalid_order() {
    printf("[TEST] Validator - previously invalid order...\n");

    obk_order_t order = {
        .order_id = 123,
        .price = 100.0,
        .quantity = 100,
        .side = 'A',
        .is_valid = false
    };

    vld_validate_order(&order, 1);

    assert(order.order_id == (uint32_t)-1);

    printf("[PASS] Previously invalid order normalized.\n\n");
}

// 9. Validate mixed buffer — one valid order survives alongside three invalid entries
void test_mixed_buffer() {
    printf("[TEST] Validator - mixed buffer...\n");

    obk_order_t orders[4] = {
        {.order_id=1,.price=100,.quantity=10,.side='A',.is_valid=true},
        {.order_id=2,.price=0,.quantity=10,.side='A',.is_valid=true},
        {.order_id=3,.price=100,.quantity=0,.side='B',.is_valid=true},
        {.order_id=4,.price=100,.quantity=10,.side='X',.is_valid=true}
    };

    vld_validate_order(orders,4);

    assert(orders[0].is_valid == true);

    assert(orders[1].is_valid == false);
    assert(orders[2].is_valid == false);
    assert(orders[3].is_valid == false);

    printf("[PASS] Mixed buffer validated correctly.\n\n");
}

int main() {

    printf("=============== STARTING VALIDATOR TESTS ===============\n\n");

    test_null_buffer();
    test_zero_count();
    test_valid_orders();
    test_invalid_price_zero();
    test_invalid_price_negative();
    test_invalid_quantity();
    test_invalid_side();
    test_previously_invalid_order();
    test_mixed_buffer();

    printf("=============== ALL VALIDATOR TESTS PASSED ===============\n");

    return 0;
}

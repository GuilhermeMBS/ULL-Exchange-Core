/*
cmd: gcc core/src/errorlib.c core/src/parser.c core/src/validator.c tests/c_tests/test_validator.c -o validator_tester -Icore/include -Wall -Wextra
*/

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include "validator.h"
#include "book.h"

/* Teste 1: Valida que vld_validate_order lida com buffer NULL
 * de forma segura, retornando imediatamente sem travar. */
void test_null_buffer() {
    printf("[TEST] Validator - NULL buffer...\n");

    vld_validate_order(NULL, 10);

    printf("[PASS] NULL buffer handled correctly.\n\n");
}

/* Teste 2: Valida que vld_validate_order não faz nada quando count = 0,
 * deixando uma ordem válida intacta. */
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

/* Teste 3: Valida que ordens estruturalmente corretas (preço positivo,
 * quantidade >= 1, lado válido) são preservadas com is_valid = true. */
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

/* Teste 4: Valida que uma ordem com preço = 0.0 é rejeitada
 * (is_valid = false, order_id = -1). */
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

/* Teste 5: Valida que uma ordem com preço negativo é rejeitada
 * (is_valid = false, order_id = -1). */
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

/* Teste 6: Valida que uma ordem com quantity = 0 é rejeitada
 * (is_valid = false, order_id = -1). */
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

/* Teste 7: Valida que uma ordem com caractere de lado não reconhecido
 * (qualquer coisa diferente de 'A' ou 'B') é rejeitada
 * (is_valid = false, order_id = -1). */
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

/* Teste 8: Valida que uma ordem já marcada como inválida pelo Parser
 * (is_valid = false) é normalizada pelo Validator: o order_id é definido
 * como -1 sem que seus campos sejam reavaliados. */
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

/* Teste 9: Valida o comportamento com um buffer misto contendo uma ordem
 * válida e três inválidas (preço zero, quantidade zero, lado inválido).
 * Apenas a primeira ordem deve permanecer válida; as demais devem ser rejeitadas. */
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
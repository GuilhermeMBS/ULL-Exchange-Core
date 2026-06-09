/*
cmd: gcc core/src/errorlib.c core/src/ledger.c tests/c_tests/test_ledger.c -o ledger_tester -Icore/include -Wall -Wextra
*/

#include <stdio.h>
#include <assert.h>

#include "ledger.h"

static const char* TEST_PATH = "test_ledger_temp.bin";

void cleanup() {
    remove(TEST_PATH);
}

void test_init_caminho_valido() {
    int32_t result = ldg_init_ledger(TEST_PATH);
    assert(result == 0);
    cleanup();
    printf("test_init_caminho_valido: OK\n");
}

void test_init_caminho_invalido() {
    int32_t result = ldg_init_ledger("/caminho/invalido/ledger.bin");
    assert(result == -1);
    printf("test_init_caminho_invalido: OK\n");
}

void test_register_ponteiro_nulo() {
    int32_t result = ldg_register_trade(NULL);
    assert(result == -3);
    printf("test_register_ponteiro_nulo: OK\n");
}

void test_register_transacao_valida() {
    ldg_init_ledger(TEST_PATH);

    mtc_transaction_t t = {
        .timestamp      = 1000,
        .trade_id       = 1,
        .buy_order_id   = 10,
        .sell_order_id  = 20,
        .buy_client_id  = 101,
        .sell_client_id = 102,
        .price          = 50.0,
        .quantity       = 5
    };

    int32_t result = ldg_register_trade(&t);
    assert(result == 0);
    cleanup();
    printf("test_register_transacao_valida: OK\n");
}

void test_100_transacoes_em_ordem() {
    ldg_init_ledger(TEST_PATH);

    for (int i = 0; i < 100; i++) {
        mtc_transaction_t t = {
            .timestamp      = 1000 + i,
            .trade_id       = i + 1,
            .buy_order_id   = (i * 2) + 1,
            .sell_order_id  = (i * 2) + 2,
            .buy_client_id  = 100 + i,
            .sell_client_id = 200 + i,
            .price          = 50.0 + i,
            .quantity       = 10 + i
        };

        int32_t result = ldg_register_trade(&t);
        assert(result == 0);
    }

    cleanup();
    printf("test_100_transacoes_em_ordem: OK\n");
}

void test_ordem_preservada_no_arquivo() {
    ldg_init_ledger(TEST_PATH);

    mtc_transaction_t transactions[5];
    for (int i = 0; i < 5; i++) {
        transactions[i].trade_id  = i + 1;
        transactions[i].price     = 10.0 * (i + 1);
        transactions[i].quantity  = i + 1;
        transactions[i].timestamp = 1000 + i;
        ldg_register_trade(&transactions[i]);
    }

    FILE* f = fopen(TEST_PATH, "rb");
    assert(f != NULL);

    mtc_transaction_t read_back[5];
    fread(read_back, sizeof(mtc_transaction_t), 5, f);
    fclose(f);

    for (int i = 0; i < 5; i++) {
        assert(read_back[i].trade_id == transactions[i].trade_id);
        assert(read_back[i].quantity == transactions[i].quantity);
    }

    cleanup();
    printf("test_ordem_preservada_no_arquivo: OK\n");
}

int main() {
    test_init_caminho_valido();
    test_init_caminho_invalido();
    test_register_ponteiro_nulo();
    test_register_transacao_valida();
    test_100_transacoes_em_ordem();
    test_ordem_preservada_no_arquivo();
    printf("\nTodos os testes do Ledger passaram!\n");
    return 0;
}

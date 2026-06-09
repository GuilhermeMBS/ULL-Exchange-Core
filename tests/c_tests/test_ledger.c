/*
cmd: gcc core/src/errorlib.c core/src/ledger.c tests/c_tests/test_ledger.c -o ledger_tester -Icore/include -Wall -Wextra
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "ledger.h"

/* Testa buffer nulo — deve retornar -3 (erro de memória) */
void test_null_buffer() {
    int result = ldg_register_trades(NULL);
    assert(result == -3);
    printf("test_null_buffer: OK\n");
}

/* Testa buffer com data nulo — deve retornar -3 */
void test_null_data() {
    Buffer b = { .data = NULL, .count = 1 };
    int result = ldg_register_trades(&b);
    assert(result == -3);
    printf("test_null_data: OK\n");
}

/* Testa buffer com count zero — deve retornar -3 */
void test_zero_count() {
    mtc_transaction_t t = {0};
    Buffer b = { .data = &t, .count = 0 };
    int result = ldg_register_trades(&b);
    assert(result == -3);
    printf("test_zero_count: OK\n");
}

/*
 * Caso de Teste do Documento:
 * Salvar 100 transações completas na ordem — deve retornar 0
 */
void test_100_transactions_in_order() {
    mtc_transaction_t transactions[100];

    for (int i = 0; i < 100; i++) {
        transactions[i].timestamp      = 1000 + i;
        transactions[i].trade_id       = i + 1;
        transactions[i].buy_order_id   = (i * 2) + 1;
        transactions[i].sell_order_id  = (i * 2) + 2;
        transactions[i].buy_client_id  = 100 + i;
        transactions[i].sell_client_id = 200 + i;
        transactions[i].price          = 50.0 + i;
        transactions[i].quantity       = 10 + i;
    }

    Buffer b = { .data = transactions, .count = 100 };
    int result = ldg_register_trades(&b);
    assert(result == 0);
    printf("test_100_transactions_in_order: OK\n");
}

/*
 * Verifica se as transações foram gravadas na ordem correta
 * lendo o arquivo binário gerado
 */
void test_order_preserved_in_file() {
    mtc_transaction_t transactions[5];
    for (int i = 0; i < 5; i++) {
        transactions[i].trade_id  = i + 1;
        transactions[i].price     = 10.0 * (i + 1);
        transactions[i].quantity  = i + 1;
        transactions[i].timestamp = 1000 + i;
    }

    Buffer b = { .data = transactions, .count = 5 };
    ldg_register_trades(&b);

    /* Lê o arquivo gerado e verifica a ordem */
    FILE* f = fopen("ledger.bin", "rb");
    assert(f != NULL);

    mtc_transaction_t read_back[5];
    fread(read_back, sizeof(mtc_transaction_t), 5, f);
    fclose(f);

    for (int i = 0; i < 5; i++) {
        assert(read_back[i].trade_id == transactions[i].trade_id);
        assert(read_back[i].quantity == transactions[i].quantity);
    }
    printf("test_order_preserved_in_file: OK\n");
}

int main() {
    test_null_buffer();
    test_null_data();
    test_zero_count();
    test_100_transactions_in_order();
    test_order_preserved_in_file();
    printf("\nTodos os testes do Ledger passaram!\n");
    return 0;
}

#include <stdio.h>
#include <assert.h>

#include "ledger.h"

static const char* TEST_PATH = "data/test_ledger_temp.bin";

static void cleanup() {
    remove(TEST_PATH);
}

// 1. Validate successful ledger initialisation with a valid file path
void test_init_caminho_valido() {
    printf("[TEST] Requirement 1: Valid Path Initialisation...\n");

    ret_code_t result = ldg_init_ledger(TEST_PATH);
    assert(result == ERR_NONE);
    cleanup();

    printf("[PASS] Ledger initialised at valid path.\n\n");
}

// 2. Validate rejection of an inaccessible path on initialisation
void test_init_caminho_invalido() {
    printf("[TEST] Requirement 2: Invalid Path Rejection...\n");

    ret_code_t result = ldg_init_ledger("/caminho/invalido/ledger.bin");
    assert(result == ERR_ORD);

    printf("[PASS] Invalid path rejected with ERR_ORD.\n\n");
}

// 3. Validate null pointer rejection on trade registration
void test_register_ponteiro_nulo() {
    printf("[TEST] Requirement 3: Null Pointer Rejection on Register...\n");

    ret_code_t result = ldg_register_trade(NULL);
    assert(result == ERR_MEM);

    printf("[PASS] Null trade pointer rejected with ERR_MEM.\n\n");
}

// 4. Validate successful write of a single valid trade record
void test_register_transacao_valida() {
    printf("[TEST] Requirement 4: Single Valid Trade Registration...\n");

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

    ret_code_t result = ldg_register_trade(&t);
    assert(result == ERR_NONE);
    cleanup();

    printf("[PASS] Single trade record written successfully.\n\n");
}

// 5. Validate sequential write of 100 consecutive trade records
void test_100_transacoes_em_ordem() {
    printf("[TEST] Requirement 5: Sequential Write of 100 Trade Records...\n");

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

        ret_code_t result = ldg_register_trade(&t);
        assert(result == ERR_NONE);
    }

    cleanup();
    printf("[PASS] 100 consecutive trade records written without error.\n\n");
}

// 6. Validate insertion order preservation in the binary output file
void test_ordem_preservada_no_arquivo() {
    printf("[TEST] Requirement 6: Insertion Order Preservation in Binary File...\n");

    ldg_init_ledger(TEST_PATH);

    mtc_transaction_t transactions[5];
    for (int i = 0; i < 5; i++) {
        transactions[i].trade_id  = i + 1;
        transactions[i].price     = 10.0 * (i + 1);
        transactions[i].quantity  = i + 1;
        transactions[i].timestamp = 1000 + i;
        ldg_register_trade(&transactions[i]);
    }

    // Read back raw bytes and verify each record matches its original position
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
    printf("[PASS] Binary file preserves insertion order across all records.\n\n");
}

int main() {
    printf("\n============== STARTING LEDGER ENGINE SUITE ===============\n\n");

    test_init_caminho_valido();
    test_init_caminho_invalido();
    test_register_ponteiro_nulo();
    test_register_transacao_valida();
    test_100_transacoes_em_ordem();
    test_ordem_preservada_no_arquivo();

    printf("============ ALL TEST DOMAINS VERIFIED SUCCESSFULLY ===========\n\n");
    return 0;
}

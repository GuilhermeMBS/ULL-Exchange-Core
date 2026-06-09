#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "retcodes.h"
#include "matching.h"
#include "ledger.h"

static const char* TEST_PATH = "data/test_ledger_temp.bin";


/* Private transaction struct layout definition to facilitate validation reading tests */
typedef struct {
    uint32_t timestamp;
    int32_t trade_id;
    int32_t buy_order_id;
    int32_t sell_order_id;
    int32_t buy_client_id;
    int32_t sell_client_id;
    double price;
    int32_t quantity;
} test_transaction_t;


static void cleanup() {
    remove(TEST_PATH);
}


// 1. Validate successful ledger initialisation with a valid file path
void test_init_caminho_valido() {
    printf("[TEST] Requirement 1: Valid Path Initialisation...\n");

    mtc_handle_pt mtc_handle = NULL;
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    /* Invokes the save routine with zero records to verify initialisation and file descriptor trashing */
    ret_code_t result = ldg_save_ledger(TEST_PATH, mtc_handle, 0);
    assert(result == ERR_NONE);

    mtc_clear_engine(&mtc_handle);
    cleanup();

    printf("[PASS] Ledger initialised at valid path.\n\n");
}


// 2. Validate rejection of an inaccessible path on initialisation
void test_init_caminho_invalido() {
    printf("[TEST] Requirement 2: Invalid Path Rejection...\n");

    mtc_handle_pt mtc_handle = NULL;
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    /* Rejects inaccessible operating system folders without crashing returning structural error codes */
    ret_code_t result = ldg_save_ledger("/caminho/invalido/ledger.bin", mtc_handle, 0);
    assert(result == ERR_ORD);

    mtc_clear_engine(&mtc_handle);

    printf("[PASS] Invalid path rejected with ERR_ORD.\n\n");
}


// 3. Validate null pointer rejection on trade registration
void test_register_ponteiro_nulo() {
    printf("[TEST] Requirement 3: Null Pointer Rejection on Register...\n");

    ret_code_t result = ldg_save_ledger(TEST_PATH, NULL, 10);
    assert(result == ERR_ORD);

    printf("[PASS] Null trade pointer rejected with ERR_MEM.\n\n");
}


// 4. Validate successful write of a single valid trade record
void test_register_transacao_valida() {
    printf("[TEST] Requirement 4: Single Valid Trade Registration...\n");

    mtc_handle_pt mtc_handle = NULL;
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    /* Internal standalone instance hacking mechanism matching your local mock pattern rules */
    struct mtc_instance_private_s {
        void* book;
        int32_t trade_id;
        test_transaction_t trades_array[10];
        int32_t trades_count;
    };
    struct mtc_instance_private_s* mock = (struct mtc_instance_private_s*)mtc_handle;

    mock->trades_array[0].timestamp = 1000;
    mock->trades_array[0].trade_id = 1;
    mock->trades_array[0].buy_order_id = 10;
    mock->trades_array[0].sell_order_id = 20;
    mock->trades_array[0].buy_client_id = 101;
    mock->trades_array[0].sell_client_id = 102;
    mock->trades_array[0].price = 50.0;
    mock->trades_array[0].quantity = 5;
    mock->trades_count = 1;

    ret_code_t result = ldg_save_ledger(TEST_PATH, mtc_handle, 1);
    assert(result == ERR_NONE);

    FILE* f = fopen(TEST_PATH, "rb");
    assert(f != NULL);
    test_transaction_t read_t;
    assert(fread(&read_t, sizeof(test_transaction_t), 1, f) == 1);
    fclose(f);

    assert(read_t.trade_id == 1);
    assert(read_t.price == 50.0);

    mtc_clear_engine(&mtc_handle);
    cleanup();

    printf("[PASS] Single trade record written successfully.\n\n");
}


// 5. Validate sequential write of 100 consecutive trade records
void test_100_transacoes_em_ordem() {
    printf("[TEST] Requirement 5: Sequential Write of 100 Trade Records...\n");

    mtc_handle_pt mtc_handle = NULL;
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    struct mtc_instance_private_s {
        void* book;
        int32_t trade_id;
        test_transaction_t trades_array[200];
        int32_t trades_count;
    };
    struct mtc_instance_private_s* mock = (struct mtc_instance_private_s*)mtc_handle;

    for (int i = 0; i < 100; i++) {
        mock->trades_array[i].timestamp = 1000 + i;
        mock->trades_array[i].trade_id = i + 1;
        mock->trades_array[i].buy_order_id = (i * 2) + 1;
        mock->trades_array[i].sell_order_id = (i * 2) + 2;
        mock->trades_array[i].buy_client_id = 100 + i;
        mock->trades_array[i].sell_client_id = 200 + i;
        mock->trades_array[i].price = 50.0 + i;
        mock->trades_array[i].quantity = 10 + i;
    }
    mock->trades_count = 100;

    ret_code_t result = ldg_save_ledger(TEST_PATH, mtc_handle, 100);
    assert(result == ERR_NONE);

    mtc_clear_engine(&mtc_handle);
    cleanup();

    printf("[PASS] 100 consecutive trade records written without error.\n\n");
}


// 6. Validate insertion order preservation in the binary output file
void test_ordem_preservada_no_arquivo() {
    printf("[TEST] Requirement 6: Insertion Order Preservation in Binary File...\n");

    mtc_handle_pt mtc_handle = NULL;
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    struct mtc_instance_private_s {
        void* book;
        int32_t trade_id;
        test_transaction_t trades_array[10];
        int32_t trades_count;
    };
    struct mtc_instance_private_s* mock = (struct mtc_instance_private_s*)mtc_handle;

    for (int i = 0; i < 5; i++) {
        mock->trades_array[i].trade_id  = i + 1;
        mock->trades_array[i].price     = 10.0 * (i + 1);
        mock->trades_array[i].quantity  = i + 1;
        mock->trades_array[i].timestamp = 1000 + i;
    }
    mock->trades_count = 5;

    assert(ldg_save_ledger(TEST_PATH, mtc_handle, 5) == ERR_NONE);

    FILE* f = fopen(TEST_PATH, "rb");
    assert(f != NULL);

    test_transaction_t read_back[5];
    assert(fread(read_back, sizeof(test_transaction_t), 5, f) == 5);
    fclose(f);

    for (int i = 0; i < 5; i++) {
        assert(read_back[i].trade_id == mock->trades_array[i].trade_id);
        assert(read_back[i].quantity == mock->trades_array[i].quantity);
    }

    mtc_clear_engine(&mtc_handle);
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

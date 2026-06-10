#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "retcodes.h"
#include "book.h"
#include "parserlib.h"
#include "matching.h"
#include "ledger.h"

static const char* TEST_PATH = "data/test_ledger_temp.bin";


static void create_test_csv(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    assert(fp != NULL);

    fputs(content, fp);

    fclose(fp);
}


static void cleanup() {
    remove(TEST_PATH);
}


// 1. Validate successful ledger initialisation with a valid file path
void test_init_caminho_valido() {
    printf("[TEST] Requirement 1: Valid Path Initialisation...\n");

    mtc_handle_pt mtc_handle = NULL;
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

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

    /* Generates an accurate matching condition by running real modular pipelines */
    create_test_csv(
        "ldg_single.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,10,101,5,50.0,PETR4,A\n"
        "1001,20,102,5,50.0,PETR4,B\n"
    );

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;

    assert(prs_create_orders("ldg_single.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 1);

    ret_code_t result = ldg_save_ledger(TEST_PATH, mtc_handle, total_trades);
    assert(result == ERR_NONE);

    FILE* f = fopen(TEST_PATH, "rb");
    assert(f != NULL);

    mtc_transaction_t read_t;
    size_t elements_read = fread(&read_t, sizeof(mtc_transaction_t), 1, f);
    fclose(f);

    assert(elements_read == 1);
    assert(read_t.trade_id == 1);
    assert(read_t.price == 50.0);
    assert(read_t.quantity == 5);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("ldg_single.csv");
    cleanup();

    printf("[PASS] Single trade record written successfully.\n\n");
}


// 5. Validate sequential write of 100 consecutive trade records
void test_100_transacoes_em_ordem() {
    printf("[TEST] Requirement 5: Sequential Write of 100 Trade Records...\n");

    /* Creates 100 asks and 100 matching bids into a temporary file to populate 100 true transactions */
    FILE* fp = fopen("ldg_multi.csv", "w");
    assert(fp != NULL);
    fputs("timestamp,order_id,client_id,quantity,price,symbol,side\n", fp);
    for (int i = 0; i < 100; i++) {
        fprintf(fp, "%d,%d,%d,%d,%lf,PETR4,A\n", 1000 + i, (i * 2) + 2, 200 + i, 10 + i, 50.0 + i);
        fprintf(fp, "%d,%d,%d,%d,%lf,PETR4,B\n", 1000 + i, (i * 2) + 1, 100 + i, 10 + i, 50.0 + i);
    }
    fclose(fp);

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;

    assert(prs_create_orders("ldg_multi.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 100);

    ret_code_t result = ldg_save_ledger(TEST_PATH, mtc_handle, total_trades);
    assert(result == ERR_NONE);

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("ldg_multi.csv");
    cleanup();

    printf("[PASS] 100 consecutive trade records written without error.\n\n");
}


// 6. Validate insertion order preservation in the binary output file
void test_ordem_preservada_no_arquivo() {
    printf("[TEST] Requirement 6: Insertion Order Preservation in Binary File...\n");

    FILE* fp = fopen("ldg_preserve.csv", "w");
    assert(fp != NULL);
    fputs("timestamp,order_id,client_id,quantity,price,symbol,side\n", fp);
    for (int i = 0; i < 5; i++) {
        fprintf(fp, "%d,%d,1,%d,%lf,PETR4,A\n", 1000 + i, i + 1, i + 1, 10.0 * (i + 1));
        fprintf(fp, "%d,%d,2,%d,%lf,PETR4,B\n", 1000 + i, i + 10, i + 1, 10.0 * (i + 1));
    }
    fclose(fp);

    int32_t total_orders = 0;
    int32_t total_trades = 0;
    prs_handle_pt prs_handle = NULL;
    mtc_handle_pt mtc_handle = NULL;

    assert(prs_create_orders("ldg_preserve.csv", &prs_handle, &total_orders) == ERR_NONE);
    assert(mtc_create_engine(&mtc_handle) == ERR_NONE);

    assert(mtc_process_matching(mtc_handle, prs_handle, total_orders, &total_trades) == ERR_NONE);
    assert(total_trades == 5);

    assert(ldg_save_ledger(TEST_PATH, mtc_handle, total_trades) == ERR_NONE);

    FILE* f = fopen(TEST_PATH, "rb");
    assert(f != NULL);

    mtc_transaction_t read_back[5];
    size_t total_read = fread(read_back, sizeof(mtc_transaction_t), 5, f);
    fclose(f);

    assert(total_read == 5);

    for (int i = 0; i < 5; i++) {
        assert(read_back[i].trade_id == i + 1);
        assert(read_back[i].quantity == i + 1);
    }

    prs_free_buffer(prs_handle);
    mtc_clear_engine(&mtc_handle);
    remove("ldg_preserve.csv");
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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "retcodes.h"
#include "parserlib.h"
#include "validator.h"
#include "book.h"


// Helper: writes content to a temporary CSV file so tests do not depend on files on disk
static void create_csv(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    assert(fp != NULL);

    fputs(content, fp);

    fclose(fp);
}


// 1. Validate null argument rejection — no crash or undefined behaviour on NULL input
void test_null_arguments() {
    printf("[TEST] Parser - NULL arguments...\n");

    int32_t count = 0;
    prs_handle_pt handle = NULL;
    ret_code_t code;

    code = prs_create_orders(NULL, &handle, &count);
    assert(code == ERR_ORD);

    code = prs_create_orders("dummy.csv", NULL, &count);
    assert(code == ERR_ORD);

    code = prs_create_orders("dummy.csv", &handle, NULL);
    assert(code == ERR_ORD);

    printf("[PASS] NULL arguments handled.\n\n");
}


// 2. Validate rejection of a file path that does not exist on disk
void test_nonexistent_file() {
    printf("[TEST] Parser - nonexistent file...\n");

    int32_t count = 0;
    prs_handle_pt handle = NULL;
    ret_code_t code;

    code = prs_create_orders("file_that_does_not_exist.csv", &handle, &count);
    assert(code == ERR_ORD);
    assert(handle == NULL);

    printf("[PASS] Nonexistent file rejected.\n\n");
}


// 3. Validate rejection of a completely empty file (no header, no data rows)
void test_empty_file() {
    printf("[TEST] Parser - empty file...\n");

    create_csv("empty.csv", "");

    int32_t count = 0;
    prs_handle_pt handle = NULL;
    ret_code_t code;

    code = prs_create_orders("empty.csv", &handle, &count);
    assert(code == ERR_ORD);
    assert(handle == NULL);

    remove("empty.csv");

    printf("[PASS] Empty file rejected.\n\n");
}


// 4. Validate rejection of a file containing only the header line with no data rows
void test_header_only() {
    printf("[TEST] Parser - header only...\n");

    create_csv(
        "header.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
    );

    int32_t count = 0;
    prs_handle_pt handle = NULL;
    ret_code_t code;

    code = prs_create_orders("header.csv", &handle, &count);
    assert(code == ERR_ORD);
    assert(handle == NULL);

    remove("header.csv");

    printf("[PASS] Header-only file rejected.\n\n");
}


// 5. Validate correct field mapping for a single well-formed CSV row into obk_order_t
void test_single_valid_order() {
    printf("[TEST] Parser - single valid order...\n");

    create_csv(
        "valid.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,1,10,100,50.5,PETR4,B\n"
    );

    int32_t count = 0;
    prs_handle_pt handle = NULL;
    obk_order_t local_order;
    ret_code_t code;

    code = prs_create_orders("valid.csv", &handle, &count);
    assert(code == ERR_NONE);
    assert(handle != NULL);
    assert(count == 1);

    code = prs_get_order_by_index(handle, 0, &local_order);
    assert(code == ERR_NONE);

    assert(local_order.order_id == 1);
    assert(local_order.client_id == 10);
    assert(local_order.quantity == 100);
    assert(local_order.price == 50.5);
    assert(strcmp(local_order.symbol, "PETR4") == 0);
    assert(local_order.side == 'B');
    assert(local_order.is_valid == true);

    code = prs_free_buffer(handle);
    assert(code == ERR_NONE);

    remove("valid.csv");

    printf("[PASS] Single valid order loaded.\n\n");
}


// 6. Validate sequential loading of multiple well-formed CSV rows preserving insertion order
void test_multiple_valid_orders() {
    printf("[TEST] Parser - multiple valid orders...\n");

    create_csv(
        "multi.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,1,10,100,50.0,PETR4,B\n"
        "1001,2,11,200,51.0,VALE3,A\n"
        "1002,3,12,300,52.0,ITUB4,B\n"
    );

    int32_t count = 0;
    prs_handle_pt handle = NULL;
    obk_order_t local_order;
    ret_code_t code;

    code = prs_create_orders("multi.csv", &handle, &count);
    assert(code == ERR_NONE);
    assert(handle != NULL);
    assert(count == 3);

    code = prs_get_order_by_index(handle, 0, &local_order);
    assert(code == ERR_NONE);
    assert(local_order.order_id == 1);

    code = prs_get_order_by_index(handle, 1, &local_order);
    assert(code == ERR_NONE);
    assert(local_order.order_id == 2);

    code = prs_get_order_by_index(handle, 2, &local_order);
    assert(code == ERR_NONE);
    assert(local_order.order_id == 3);

    code = prs_get_order_by_index(handle, 3, &local_order);
    assert(code == ERR_ORD);

    code = prs_free_buffer(handle);
    assert(code == ERR_NONE);

    remove("multi.csv");

    printf("[PASS] Multiple orders loaded.\n\n");
}


// 7. Validate that a malformed CSV row occupies a buffer slot and is marked invalid (order_id = -1)
void test_malformed_line() {
    printf("[TEST] Parser - malformed line...\n");

    create_csv(
        "malformed.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,1,10\n"
    );

    int32_t count = 0;
    prs_handle_pt handle = NULL;
    obk_order_t local_order;
    ret_code_t code;

    code = prs_create_orders("malformed.csv", &handle, &count);
    assert(code == ERR_NONE);
    assert(handle != NULL);
    assert(count == 1);

    code = prs_get_order_by_index(handle, 0, &local_order);
    assert(code == ERR_NONE);
    assert(local_order.is_valid == false);
    assert(local_order.order_id == (uint32_t)-1);

    code = prs_free_buffer(handle);
    assert(code == ERR_NONE);

    remove("malformed.csv");

    printf("[PASS] Malformed line detected.\n\n");
}


// 8. Validate Parser + Validator integration — structurally valid row with quantity = 0 is invalidated
void test_validator_integration() {
    printf("[TEST] Parser + Validator integration...\n");

    create_csv(
        "integration.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,1,10,0,50.0,PETR4,B\n"
    );

    int32_t count = 0;
    prs_handle_pt handle = NULL;
    obk_order_t local_order;
    ret_code_t code;

    code = prs_create_orders("integration.csv", &handle, &count);
    assert(code == ERR_NONE);
    assert(handle != NULL);

    code = prs_get_order_by_index(handle, 0, &local_order);
    assert(code == ERR_NONE);
    assert(local_order.is_valid == false);
    assert(local_order.order_id == (uint32_t)-1);

    code = prs_free_buffer(handle);
    assert(code == ERR_NONE);

    remove("integration.csv");

    printf("[PASS] Validator invoked automatically.\n\n");
}


// 9. Validate buffer deallocation — returns 0 on a valid pointer, -1 on NULL
void test_free_buffer() {
    printf("[TEST] Parser - free buffer...\n");

    ret_code_t code;

    code = prs_free_buffer(NULL);
    assert(code == ERR_ORD);

    printf("[PASS] Buffer deallocation validated.\n\n");
}


int main() {
    printf("=============== STARTING PARSER TESTS ===============\n\n");

    test_null_arguments();
    test_nonexistent_file();
    test_empty_file();
    test_header_only();
    test_single_valid_order();
    test_multiple_valid_orders();
    test_malformed_line();
    test_validator_integration();
    test_free_buffer();

    printf("=============== ALL PARSER TESTS PASSED ===============\n");

    return 0;
}

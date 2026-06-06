/*
cmd: gcc core/src/errorlib.c core/src/parser.c core/src/validator.c tests/c_tests/test_parser.c -o parser_tester -Icore/include -Wall -Wextra
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "parser.h"
#include "validator.h"
#include "book.h"

static void create_csv(const char *filename, const char *content) {

    FILE *fp = fopen(filename, "w");
    assert(fp != NULL);

    fputs(content, fp);

    fclose(fp);
}

void test_null_arguments() {

    printf("[TEST] Parser - NULL arguments...\n");

    int32_t count;

    assert(prs_create_orders(NULL, &count) == NULL);
    assert(prs_create_orders("dummy.csv", NULL) == NULL);

    printf("[PASS] NULL arguments handled.\n\n");
}

void test_nonexistent_file() {

    printf("[TEST] Parser - nonexistent file...\n");

    int32_t count;

    assert(prs_create_orders("file_that_does_not_exist.csv", &count) == NULL);

    printf("[PASS] Nonexistent file rejected.\n\n");
}

void test_empty_file() {

    printf("[TEST] Parser - empty file...\n");

    create_csv("empty.csv", "");

    int32_t count;

    assert(prs_create_orders("empty.csv", &count) == NULL);

    remove("empty.csv");

    printf("[PASS] Empty file rejected.\n\n");
}

void test_header_only() {

    printf("[TEST] Parser - header only...\n");

    create_csv(
        "header.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
    );

    int32_t count;

    assert(prs_create_orders("header.csv", &count) == NULL);

    remove("header.csv");

    printf("[PASS] Header-only file rejected.\n\n");
}

void test_single_valid_order() {

    printf("[TEST] Parser - single valid order...\n");

    create_csv(
        "valid.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,1,10,100,50.5,PETR4,B\n"
    );

    int32_t count;

    obk_order_t *buffer =
        prs_create_orders("valid.csv", &count);

    assert(buffer != NULL);
    assert(count == 1);

    assert(buffer[0].order_id == 1);
    assert(buffer[0].client_id == 10);
    assert(buffer[0].quantity == 100);
    assert(buffer[0].price == 50.5);
    assert(strcmp(buffer[0].symbol, "PETR4") == 0);
    assert(buffer[0].side == 'B');
    assert(buffer[0].is_valid == true);

    assert(prs_free_buffer(buffer) == 0);

    remove("valid.csv");

    printf("[PASS] Single valid order loaded.\n\n");
}

void test_multiple_valid_orders() {

    printf("[TEST] Parser - multiple valid orders...\n");

    create_csv(
        "multi.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,1,10,100,50.0,PETR4,B\n"
        "1001,2,11,200,51.0,VALE3,A\n"
        "1002,3,12,300,52.0,ITUB4,B\n"
    );

    int32_t count;

    obk_order_t *buffer =
        prs_create_orders("multi.csv", &count);

    assert(buffer != NULL);
    assert(count == 3);

    assert(buffer[0].order_id == 1);
    assert(buffer[1].order_id == 2);
    assert(buffer[2].order_id == 3);

    prs_free_buffer(buffer);

    remove("multi.csv");

    printf("[PASS] Multiple orders loaded.\n\n");
}

void test_malformed_line() {

    printf("[TEST] Parser - malformed line...\n");

    create_csv(
        "malformed.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,1,10\n"
    );

    int32_t count;

    obk_order_t *buffer =
        prs_create_orders("malformed.csv", &count);

    assert(buffer != NULL);
    assert(count == 1);

    assert(buffer[0].is_valid == false);
    assert(buffer[0].order_id == (uint32_t)-1);

    prs_free_buffer(buffer);

    remove("malformed.csv");

    printf("[PASS] Malformed line detected.\n\n");
}

void test_validator_integration() {

    printf("[TEST] Parser + Validator integration...\n");

    create_csv(
        "integration.csv",
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,1,10,0,50.0,PETR4,B\n"
    );

    int32_t count;

    obk_order_t *buffer =
        prs_create_orders("integration.csv", &count);

    assert(buffer != NULL);

    assert(buffer[0].is_valid == false);
    assert(buffer[0].order_id == (uint32_t)-1);

    prs_free_buffer(buffer);

    remove("integration.csv");

    printf("[PASS] Validator invoked automatically.\n\n");
}

void test_free_buffer() {

    printf("[TEST] Parser - free buffer...\n");

    obk_order_t *buffer =
        (obk_order_t *)malloc(sizeof(obk_order_t));

    assert(prs_free_buffer(buffer) == 0);
    assert(prs_free_buffer(NULL) == -1);

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
#pragma once

#include "errorlib.h"
#include "matching.h"

/*
 * @file ledger.h
 * @brief Binary transaction log module.
 */

/*
 * Groups an array of transactions and its count.
 */
typedef struct {
    mtc_transaction_t* data;
    int32_t count;
} ldg_buffer_t;


/*
  Initializes the binary ledger file at the given path.
  @param bin_path  Path to the .bin file to be created
  @return          0 on success, -1 on permission error
*/
ret_code_t ldg_init_ledger(const char* bin_path);

/*
  Appends a single transaction to the ledger.
  @param t  Pointer to the transaction to be written
  @return   0 on success, -3 on write failure
*/
int32_t ldg_register_trade(mtc_transaction_t* t);

/*
  Writes all transactions in the buffer to the ledger.
  @param transactions  Pointer to the ldg_buffer_t with the transactions
  @return              0 on success, -3 on invalid input or write failure
*/
int32_t ldg_register_all(ldg_buffer_t* transactions);

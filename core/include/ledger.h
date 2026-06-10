#pragma once

#include "retcodes.h"
#include "matching.h"


/**
 * @brief Collects and processes executed transaction entries to store them into the binary file database.
 * @param bin_path File path target where the output .bin ledger file will be generated.
 * @param mtc_handle Opaque pointer targeting the populated matching engine instance metadata container.
 * @param total_trades The total count of verified transaction items captured during execution.
 * @return ERR_NONE on successful storage execution, or ERR_MEM if file access or storage fails.
 */
ret_code_t ldg_save_ledger(const char* bin_path, mtc_handle_pt mtc_handle, int32_t total_trades);

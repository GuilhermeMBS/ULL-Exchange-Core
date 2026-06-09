#include <stdio.h>
#include <string.h>

#include "ledger.h"
#include "matching.h"


ret_code_t ldg_save_ledger(const char* bin_path, mtc_handle_pt mtc_handle, int32_t total_trades) {
    if (!bin_path || !mtc_handle) return ERR_ORD;

    FILE* f = fopen(bin_path, "wb");
    if (!f) return ERR_ORD;

    for (int32_t i = 0; i < total_trades; i++) {
        mtc_transaction_t current_trade;
        
        if (mtc_get_trade_by_index(mtc_handle, i, &current_trade) != ERR_NONE) continue;

        if (fwrite(&current_trade, sizeof(mtc_transaction_t), 1, f) != 1) {
            fclose(f);
            return ERR_MEM;
        }
    }

    fflush(f);
    fclose(f);
    return ERR_NONE;
}

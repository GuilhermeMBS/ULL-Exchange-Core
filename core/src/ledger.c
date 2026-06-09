#include <stdio.h>
#include <string.h>
#include "ledger.h"

static char ledger_path[256] = "data/ledger.bin";

ret_code_t ldg_init_ledger(const char* bin_path) {
    FILE* f = fopen(bin_path, "wb");
    if (!f) return ERR_ORD;

    strncpy(ledger_path, bin_path, sizeof(ledger_path) - 1);

    fclose(f);
    return ERR_NONE;
}

ret_code_t ldg_register_trade(mtc_transaction_t* t) {
    if (!t) return ERR_MEM;

    FILE* f = fopen(ledger_path, "ab");
    if (!f) return ERR_MEM;

    size_t written = fwrite(t, sizeof(mtc_transaction_t), 1, f);
    fclose(f);

    if (written != 1) return ERR_MEM;

    return ERR_NONE;
}
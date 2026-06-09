// ledger.c - Binary transaction log implementation.

#include <stdio.h>
#include <string.h>
#include "ledger.h"

#define LDG_PATH_MAX 256

static char s_ledger_path[LDG_PATH_MAX] = "ledger.bin";

int32_t ldg_init_ledger(const char* bin_path) {
    if (!bin_path) return -1;

    FILE* f = fopen(bin_path, "wb");
    if (!f) return -1;
    fclose(f);

    strncpy(s_ledger_path, bin_path, LDG_PATH_MAX - 1);
    s_ledger_path[LDG_PATH_MAX - 1] = '\0';

    return 0;
}

int32_t ldg_register_trade(mtc_transaction_t* t) {
    if (!t) return -3;

    FILE* f = fopen(s_ledger_path, "ab");
    if (!f) return -3;

    size_t written = fwrite(t, sizeof(mtc_transaction_t), 1, f);
    fclose(f);

    if (written != 1) return -3;

    return ERR_NONE;
}

int32_t ldg_register_all(ldg_buffer_t* transactions) {
    if (!transactions || !transactions->data || transactions->count <= 0) return -3;

    for (int32_t i = 0; i < transactions->count; i++) {
        int32_t r = ldg_register_trade(&transactions->data[i]);
        if (r != ERR_NONE) return r;
    }

    return ERR_NONE;
}

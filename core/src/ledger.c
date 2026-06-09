// ledger.c - Binary transaction log implementation.

#include <stdio.h>
#include <string.h>
#include "ledger.h"

static const char* LEDGER_PATH = "data/ledger.bin";

ret_code_t ldg_init_ledger(const char* bin_path) {
    FILE* f = fopen(bin_path, "wb");
    if (!f) return -1;
    fclose(f);

    return ERR_NONE;
}

ret_code_t ldg_register_trade(mtc_transaction_t* t) {
    /* Valida se o ponteiro é válido */
    if (!t) return ERR_MEM;

    /* Abre o arquivo em modo append para não sobrescrever */
    FILE* f = fopen(LEDGER_PATH, "ab");
    if (!f) return ERR_MEM;

    size_t written = fwrite(t, sizeof(mtc_transaction_t), 1, f);
    fclose(f);

    if (written != 1) return ERR_MEM;

    return ERR_NONE;
}

ret_code_t ldg_register_all(ldg_buffer_t* transactions) {
    if (!transactions || !transactions->data || transactions->count <= 0) return -3;

    for (int32_t i = 0; i < transactions->count; i++) {
        int32_t r = ldg_register_trade(&transactions->data[i]);
        if (r != ERR_NONE) return r;
    }

    return ERR_NONE;
}

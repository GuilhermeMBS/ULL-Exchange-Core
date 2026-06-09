// ledger.c - Implementação do registro de transações em arquivo binário (ledger.bin).

#include <stdio.h>
#include "ledger.h"

static const char* LEDGER_PATH = "ledger.bin";

int32_t ldg_init_ledger(const char* bin_path) {
    FILE* f = fopen(bin_path, "wb");
    if (!f) return -1;
    fclose(f);
    return 0;
}

int32_t ldg_register_trade(mtc_transaction_t* t) {
    /* Valida se o ponteiro é válido */
    if (!t) return -3;

    /* Abre o arquivo em modo append para não sobrescrever */
    FILE* f = fopen(LEDGER_PATH, "ab");
    if (!f) return -3;

    /* Grava a transação */
    size_t written = fwrite(t, sizeof(mtc_transaction_t), 1, f);
    fclose(f);

    if (written != 1) return -3;

    return ERR_NONE;
}
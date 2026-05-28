#include <stdio.h>
#include <ledger.h>

int registerTrades(Buffer* transactions) {
    if (!transactions || !transactions->data || transactions->count <= 0)
        return -3;

    FILE* f = fopen("ledger.bin", "wb");
    if (!f) return -3;

    size_t written = fwrite(transactions->data,
                            sizeof(Transaction),
                            transactions->count,
                            f);
    fclose(f);

    if (written != (size_t)transactions->count)
        return -3;

    return 0;
}
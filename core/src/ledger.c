#include <stdio.h>
#include <ledger.h>

int registerTrades(Buffer* transactions) {
    if (!transactions || !transactions->data || transactions->count <= 0) //  se receber algo nulo ou vazio retorna o erro
        return -3;

    FILE* f = fopen("ledger.bin", "wb");
    if (!f) return -3;

    size_t escrito = fwrite(transactions->data,
                            sizeof(Transaction),
                            transactions->count,
                            f);
    fclose(f);

    if (escrito != (size_t)transactions->count)
        return -3;

    return 0;
}
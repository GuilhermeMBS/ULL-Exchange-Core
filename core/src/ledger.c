// ledger.c - Implementação do registro de transações em arquivo binário (ledger.bin).

#include <stdio.h>
#include <ledger.h>

/**
 * Grava todas as transações do buffer no arquivo "ledger.bin".
 *
 * @param transactions  Ponteiro para o Buffer com as transações
 * @return              0 se sucesso, -3 se entrada inválida ou falha de escrita
 */

int registerTrades(Buffer* transactions) {
     /* Valida se o buffer e seus dados são válidos */
    if (!transactions || !transactions->data || transactions->count <= 0)
        return -3;

    /* Abre o arquivo binário para escrita */
    FILE* f = fopen("ledger.bin", "wb");
    if (!f) return -3;

    /* Grava todas as transações de uma vez */
    size_t written = fwrite(transactions->data,
                            sizeof(mtc_transaction_t),
                            transactions->count,
                            f);
    fclose(f);

    /* Verifica se todas as transções foram gravadas */
    if (written != (size_t)transactions->count)
        return -3;

    return 0;
}
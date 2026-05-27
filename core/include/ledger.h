#ifndef LEDGER_H
#define LEDGER_H

#include <common.h>  // Transaction definido aqui

typedef struct {
    Transaction* data;
    int count;
} Buffer;

int registerTrades(Buffer* transactions);

#endif
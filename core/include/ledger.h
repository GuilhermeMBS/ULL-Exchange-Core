#ifndef LEDGER_H
#define LEDGER_H

#include "common.h" 

typedef struct {
    int bid_id;
    int ask_id;
    double price;
    int quantity;
} Transaction;

typedef struct {
    Transaction* data;
    int count;
} Buffer;

int registerTrades(Buffer* transactions);

#endif
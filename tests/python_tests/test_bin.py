"""
test_ledger.py — Integrity validator for ledger.bin

Cross-references each transaction recorded in the ledger with the original CSV orders
and tracks found inconsistencies.

Usage:
    python3 tests/python_tests/test_bin.py
    python3 tests/python_tests/test_bin.py --csv data/market.csv --ledger data/ledger.bin
"""

import csv
import ctypes
import os
import sys
import argparse

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))
from engine_wrapper import MtcTransaction


def load_orders(csv_path):
    """Reads the CSV file and returns an order_id → order dictionary."""
    orders = {}
    with open(csv_path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            oid = int(row['order_id'])
            orders[oid] = {
                'order_id':  oid,
                'client_id': int(row['client_id']),
                'quantity':  int(row['quantity']),
                'price':     float(row['price']),
                'side':      row['side'],
            }
    return orders


def load_trades(ledger_path):
    """Reads ledger.bin and returns a list of MtcTransaction structures."""
    size = ctypes.sizeof(MtcTransaction)
    trades = []
    with open(ledger_path, 'rb') as f:
        while True:
            data = f.read(size)
            if len(data) < size:
                break
            trades.append(MtcTransaction.from_buffer_copy(data))
    return trades


def verify(trades, orders):
    errors   = []
    warnings = []

    seen_ids = {}

    for t in trades:

        # 1. Unique and positive trade_id
        if t.trade_id <= 0:
            errors.append(f"[trade {t.trade_id}] invalid trade_id ({t.trade_id})")
        if t.trade_id in seen_ids:
            errors.append(f"[trade {t.trade_id}] duplicate trade_id")
        seen_ids[t.trade_id] = True

        # 2. Positive price and quantity
        if t.price <= 0:
            errors.append(f"[trade {t.trade_id}] invalid price ({t.price:.2f})")
        if t.quantity <= 0:
            errors.append(f"[trade {t.trade_id}] invalid quantity ({t.quantity})")

        # 3. Valid order_ids (must not be flagged as -1)
        if t.buy_order_id < 0:
            errors.append(f"[trade {t.trade_id}] invalid buy_order_id ({t.buy_order_id})")
        if t.sell_order_id < 0:
            errors.append(f"[trade {t.trade_id}] invalid sell_order_id ({t.sell_order_id})")

        if t.buy_order_id < 0 or t.sell_order_id < 0:
            continue   # no source layout information available to cross-reference

        # 4. Target orders exist in the source CSV database
        buy  = orders.get(t.buy_order_id)
        sell = orders.get(t.sell_order_id)

        if buy is None:
            errors.append(f"[trade {t.trade_id}] buy_order_id {t.buy_order_id} not found in CSV mapping")
        if sell is None:
            errors.append(f"[trade {t.trade_id}] sell_order_id {t.sell_order_id} not found in CSV mapping")

        if buy is None or sell is None:
            continue

        # 5. Matching parameter side validation constraints
        if buy['side'] != 'B':
            errors.append(f"[trade {t.trade_id}] order {t.buy_order_id} should be Bid but is '{buy['side']}'")
        if sell['side'] != 'A':
            errors.append(f"[trade {t.trade_id}] order {t.sell_order_id} should be Ask but is '{sell['side']}'")

        # 6. Valid price cross boundaries evaluation (bid >= execution_price >= ask)
        if buy['price'] < t.price:
            errors.append(
                f"[trade {t.trade_id}] execution price {t.price:.2f} "
                f"above bid price threshold {buy['price']:.2f} (order {t.buy_order_id})"
            )
        if sell['price'] > t.price:
            errors.append(
                f"[trade {t.trade_id}] execution price {t.price:.2f} "
                f"below ask price threshold {sell['price']:.2f} (order {t.sell_order_id})"
            )

        # 7. Executed volume does not exceed original order book sizes
        if t.quantity > buy['quantity']:
            warnings.append(
                f"[trade {t.trade_id}] traded quantity {t.quantity} "
                f"exceeds original buy allocation {buy['quantity']} (order {t.buy_order_id})"
            )
        if t.quantity > sell['quantity']:
            warnings.append(
                f"[trade {t.trade_id}] traded quantity {t.quantity} "
                f"exceeds original sell allocation {sell['quantity']} (order {t.sell_order_id})"
            )

        # 8. Independent participant checking (prevents internal wash trading anomalies)
        if t.buy_client_id == t.sell_client_id:
            warnings.append(
                f"[trade {t.trade_id}] wash trading behavior detected "
                f"(client {t.buy_client_id} executed transaction with themselves)"
            )

    return errors, warnings


def report(trades, orders, errors, warnings):
    total   = len(trades)
    vol_tot = sum(t.quantity for t in trades if t.quantity > 0)
    valid   = sum(1 for t in trades if t.price > 0 and t.quantity > 0
                  and t.buy_order_id >= 0 and t.sell_order_id >= 0)

    print("=" * 60)
    print("  LEDGER DATABASE INTEGRITY VERIFICATION REPORT")
    print("=" * 60)
    print(f"  CSV parsed entries       : {len(orders)}")
    print(f"  Ledger total records     : {total}")
    print(f"  Valid transaction units  : {valid}")
    print(f"  Aggregated traded volume : {vol_tot}")
    print(f"  Discovered errors        : {len(errors)}")
    print(f"  Discovered warnings      : {len(warnings)}")
    print("=" * 60)

    if errors:
        print("\n[ERRORS FOUND]")
        for e in errors:
            print(f"  ✗ {e}")

    if warnings:
        print("\n[SYSTEM WARNINGS]")
        for w in warnings:
            print(f"  ! {w}")

    if not errors and not warnings:
        print("\n  Ledger integrity secure — zero inconsistencies discovered.")

    print()


def main():
    parser = argparse.ArgumentParser(description="Verifies structural data integrity of ledger.bin")
    parser.add_argument('--csv',    default='data/market.csv',  help='File path pointing to the input orders CSV')
    parser.add_argument('--ledger', default='data/ledger.bin',  help='File path pointing to the input binary ledger')
    args = parser.parse_args()

    if not os.path.exists(args.csv):
        print(f"Error: Orders CSV not found at target tracking location '{args.csv}'"); sys.exit(1)
    if not os.path.exists(args.ledger):
        print(f"Error: Binary ledger registry database not found at '{args.ledger}'"); sys.exit(1)

    orders = load_orders(args.csv)
    trades = load_trades(args.ledger)
    errors, warnings = verify(trades, orders)
    report(trades, orders, errors, warnings)

    sys.exit(1 if errors else 0)


if __name__ == '__main__':
    main()

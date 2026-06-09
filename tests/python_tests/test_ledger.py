"""
test_ledger.py — Verificador de integridade do ledger.bin

Cruza cada transação registrada no ledger com as ordens originais do CSV
e reporta inconsistências encontradas.

Uso:
    python3 tests/python_tests/test_ledger.py
    python3 tests/python_tests/test_ledger.py --csv data/market.csv --ledger data/ledger.bin
"""

import csv
import ctypes
import os
import sys
import argparse

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))
from engine_wrapper import MtcTransaction

def load_orders(csv_path):
    """Lê o CSV e retorna um dicionário order_id → ordem."""
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
    """Lê o ledger.bin e retorna uma lista de MtcTransaction."""
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

        # 1. trade_id único e positivo
        if t.trade_id <= 0:
            errors.append(f"[trade {t.trade_id}] trade_id inválido ({t.trade_id})")
        if t.trade_id in seen_ids:
            errors.append(f"[trade {t.trade_id}] trade_id duplicado")
        seen_ids[t.trade_id] = True

        # 2. Preço e quantidade positivos
        if t.price <= 0:
            errors.append(f"[trade {t.trade_id}] preço inválido ({t.price:.2f})")
        if t.quantity <= 0:
            errors.append(f"[trade {t.trade_id}] quantidade inválida ({t.quantity})")

        # 3. order_ids válidos (não devem ser -1)
        if t.buy_order_id < 0:
            errors.append(f"[trade {t.trade_id}] buy_order_id inválido ({t.buy_order_id})")
        if t.sell_order_id < 0:
            errors.append(f"[trade {t.trade_id}] sell_order_id inválido ({t.sell_order_id})")

        if t.buy_order_id < 0 or t.sell_order_id < 0:
            continue   # sem dados de origem para cruzar

        # 4. Ordens existem no CSV
        buy  = orders.get(t.buy_order_id)
        sell = orders.get(t.sell_order_id)

        if buy is None:
            errors.append(f"[trade {t.trade_id}] buy_order_id {t.buy_order_id} não encontrado no CSV")
        if sell is None:
            errors.append(f"[trade {t.trade_id}] sell_order_id {t.sell_order_id} não encontrado no CSV")

        if buy is None or sell is None:
            continue

        # 5. Lados corretos
        if buy['side'] != 'B':
            errors.append(f"[trade {t.trade_id}] ordem {t.buy_order_id} deveria ser Bid mas é '{buy['side']}'")
        if sell['side'] != 'A':
            errors.append(f"[trade {t.trade_id}] ordem {t.sell_order_id} deveria ser Ask mas é '{sell['side']}'")

        # 6. Cruzamento de preço válido (bid >= exec >= ask)
        if buy['price'] < t.price:
            errors.append(
                f"[trade {t.trade_id}] preço de execução {t.price:.2f} "
                f"acima do bid {buy['price']:.2f} (ordem {t.buy_order_id})"
            )
        if sell['price'] > t.price:
            errors.append(
                f"[trade {t.trade_id}] preço de execução {t.price:.2f} "
                f"abaixo do ask {sell['price']:.2f} (ordem {t.sell_order_id})"
            )

        # 7. Quantidade não excede nenhum dos dois lados
        if t.quantity > buy['quantity']:
            warnings.append(
                f"[trade {t.trade_id}] quantidade negociada {t.quantity} "
                f"excede a compra original {buy['quantity']} (ordem {t.buy_order_id})"
            )
        if t.quantity > sell['quantity']:
            warnings.append(
                f"[trade {t.trade_id}] quantidade negociada {t.quantity} "
                f"excede a venda original {sell['quantity']} (ordem {t.sell_order_id})"
            )

        # 8. Clientes diferentes (sem auto-trade)
        if t.buy_client_id == t.sell_client_id:
            warnings.append(
                f"[trade {t.trade_id}] auto-trade detectado "
                f"(cliente {t.buy_client_id} comprou e vendeu para si mesmo)"
            )

    return errors, warnings

def report(trades, orders, errors, warnings):
    total   = len(trades)
    vol_tot = sum(t.quantity for t in trades if t.quantity > 0)
    valid   = sum(1 for t in trades if t.price > 0 and t.quantity > 0
                  and t.buy_order_id >= 0 and t.sell_order_id >= 0)

    print("=" * 60)
    print("  RELATÓRIO DE VERIFICAÇÃO DO LEDGER")
    print("=" * 60)
    print(f"  Ordens no CSV          : {len(orders)}")
    print(f"  Transações no ledger   : {total}")
    print(f"  Transações válidas     : {valid}")
    print(f"  Volume total negociado : {vol_tot}")
    print(f"  Erros encontrados      : {len(errors)}")
    print(f"  Avisos                 : {len(warnings)}")
    print("=" * 60)

    if errors:
        print("\n[ERROS]")
        for e in errors:
            print(f"  ✗ {e}")

    if warnings:
        print("\n[AVISOS]")
        for w in warnings:
            print(f"  ! {w}")

    if not errors and not warnings:
        print("\n  Ledger íntegro — nenhuma inconsistência encontrada.")

    print()

def main():
    parser = argparse.ArgumentParser(description="Verifica a integridade do ledger.bin")
    parser.add_argument('--csv',    default='data/market.csv',  help='Caminho do CSV de ordens')
    parser.add_argument('--ledger', default='data/ledger.bin',  help='Caminho do ledger binário')
    args = parser.parse_args()

    if not os.path.exists(args.csv):
        print(f"Erro: CSV não encontrado em '{args.csv}'"); sys.exit(1)
    if not os.path.exists(args.ledger):
        print(f"Erro: ledger não encontrado em '{args.ledger}'"); sys.exit(1)

    orders = load_orders(args.csv)
    trades = load_trades(args.ledger)
    errors, warnings = verify(trades, orders)
    report(trades, orders, errors, warnings)

    sys.exit(1 if errors else 0)

if __name__ == '__main__':
    main()

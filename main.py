# main.py - Ponto de entrada do programa

import sys
from scripts.generate_market import generateMarket
from engine_wrapper import load_engine
import ctypes

# ── Configurações ─────────────────────────────────────────────────────────────

MARKET_PATH  = "data/market.csv"
LEDGER_PATH  = "data/ledger.bin"
NUM_ORDERS   = 1000
FAILURES     = 0.1

# ── Funções principais ────────────────────────────────────────────────────────

def startEngine():
    """Orquestra o fluxo completo do exchange engine."""

    #Gera o mercado (CSV)
    result = generateMarket(MARKET_PATH, num_orders=NUM_ORDERS, failures=FAILURES)
    if result != 0:
        print(f"Erro ao gerar mercado: {result}")
        return result

    #Carrega o engine C
    lib = load_engine()
    if lib is None:
        print("Erro ao carregar engine.")
        return -1

    #Inicializa o ledger
    result = lib.ldg_init_ledger(LEDGER_PATH.encode())
    if result != 0:
        print(f"Erro ao inicializar ledger: {result}")
        return result

    #Parseia o CSV em ordens C
    total_count = ctypes.c_int32(0)
    orders = lib.prs_create_orders(MARKET_PATH.encode(), ctypes.byref(total_count))
    if not orders:
        print("Erro ao parsear ordens.")
        return -1
    print(f"{total_count.value} ordens carregadas.")

    #Processa cada ordem no matching engine
    trades = 0
    for i in range(total_count.value):
        result = lib.mtc_make_trade(ctypes.byref(orders[i]))
        if result == 1 or result == 2:  # Match Total ou Parcial
            trades += 1

    print(f"Matching concluído. {trades} trades executados.")

    # 6. Libera buffer de ordens
    lib.prs_free_buffer(orders)

    print(f"Ledger salvo em: {LEDGER_PATH}")
    return 0


if __name__ == "__main__":
    result = startEngine()
    print(f"\nPrograma encerrado com código: {result}")
    sys.exit(result)

# test_wrapper.py - Testes de integração do engine C via ctypes

import ctypes
import os
import sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from engine_wrapper import load_engine, ObkOrder

LEDGER_PATH = "tests/python_tests/test_ledger_temp.bin"
MARKET_PATH = "tests/python_tests/test_market_temp.csv"

def cleanup():
    for f in [LEDGER_PATH, MARKET_PATH, "engine.so"]:
        if os.path.exists(f):
            os.remove(f)

def make_order(order_id, price, quantity, side, timestamp=1000):
    """Cria uma obk_order_t preenchida."""
    o = ObkOrder()
    o.order_id  = order_id
    o.client_id = 1
    o.timestamp = timestamp
    o.price     = price
    o.quantity  = quantity
    o.side      = side.encode()
    o.symbol    = b"PETR4"
    o.is_valid  = True
    return o

# ── Carregamento ──────────────────────────────────────────────────────────────

def test_engine_carrega():
    """Engine deve compilar e carregar sem erros."""
    lib = load_engine()
    assert lib is not None, "Falha ao carregar engine"
    print("test_engine_carrega: OK")
    return lib

# ── Ledger ────────────────────────────────────────────────────────────────────

def test_ledger_init(lib):
    """Ledger deve inicializar o arquivo binário com sucesso."""
    result = lib.ldg_init_ledger(LEDGER_PATH.encode())
    assert result == 0, f"Esperado 0, got {result}"
    assert os.path.exists(LEDGER_PATH), "Arquivo do ledger não foi criado"
    print("test_ledger_init: OK")

def test_ledger_init_caminho_invalido(lib):
    """Ledger deve retornar -1 para caminho inválido."""
    result = lib.ldg_init_ledger(b"/caminho/invalido/ledger.bin")
    assert result == -1, f"Esperado -1, got {result}"
    print("test_ledger_init_caminho_invalido: OK")

# ── Validator ─────────────────────────────────────────────────────────────────

def test_validator_ordem_invalida_preco(lib):
    """Ordem com preço <= 0 deve ter order_id alterado para -1."""
    orders = (ObkOrder * 1)()
    orders[0] = make_order(order_id=1, price=-10.0, quantity=5, side='C')
    lib.vld_validate_order(orders, 1)
    assert orders[0].order_id == ctypes.c_uint32(-1).value
    print("test_validator_ordem_invalida_preco: OK")

def test_validator_ordem_invalida_quantidade(lib):
    """Ordem com quantidade <= 0 deve ter order_id alterado para -1."""
    orders = (ObkOrder * 1)()
    orders[0] = make_order(order_id=2, price=10.0, quantity=0, side='C')
    lib.vld_validate_order(orders, 1)
    assert orders[0].order_id == ctypes.c_uint32(-1).value
    print("test_validator_ordem_invalida_quantidade: OK")

def test_validator_ordem_invalida_side(lib):
    """Ordem com side diferente de C ou V deve ter order_id alterado para -1."""
    orders = (ObkOrder * 1)()
    orders[0] = make_order(order_id=3, price=10.0, quantity=5, side='X')
    lib.vld_validate_order(orders, 1)
    assert orders[0].order_id == ctypes.c_uint32(-1).value
    print("test_validator_ordem_invalida_side: OK")

def test_validator_ordem_valida(lib):
    """Ordem válida não deve ter order_id alterado."""
    orders = (ObkOrder * 1)()
    orders[0] = make_order(order_id=4, price=10.0, quantity=5, side='C')
    lib.vld_validate_order(orders, 1)
    assert orders[0].order_id == 4
    print("test_validator_ordem_valida: OK")

# ── Matching ──────────────────────────────────────────────────────────────────

def test_matching_sem_par_vai_para_book(lib):
    """Ordem de compra sem venda disponível deve retornar 0 (foi para o book)."""
    o = make_order(order_id=10, price=50.0, quantity=5, side='C')
    result = lib.mtc_make_trade(ctypes.byref(o))
    assert result == 0, f"Esperado 0, got {result}"
    print("test_matching_sem_par_vai_para_book: OK")

def test_matching_ordem_invalida(lib):
    """Ordem inválida deve retornar -1."""
    o = make_order(order_id=ctypes.c_uint32(-1).value, price=-1.0, quantity=0, side='X')
    o.is_valid = False
    result = lib.mtc_make_trade(ctypes.byref(o))
    assert result == -1, f"Esperado -1, got {result}"
    print("test_matching_ordem_invalida: OK")

def test_matching_match_total(lib):
    """Compra e venda com mesmo preço e quantidade deve gerar match total (1)."""
    venda  = make_order(order_id=20, price=50.0, quantity=5, side='V', timestamp=1000)
    compra = make_order(order_id=21, price=50.0, quantity=5, side='C', timestamp=1001)
    lib.mtc_make_trade(ctypes.byref(venda))
    result = lib.mtc_make_trade(ctypes.byref(compra))
    assert result == 1, f"Esperado 1 (match total), got {result}"
    print("test_matching_match_total: OK")

def test_matching_match_parcial(lib):
    """Compra maior que venda deve gerar match parcial (2)."""
    venda  = make_order(order_id=30, price=50.0, quantity=3, side='V', timestamp=1000)
    compra = make_order(order_id=31, price=50.0, quantity=10, side='C', timestamp=1001)
    lib.mtc_make_trade(ctypes.byref(venda))
    result = lib.mtc_make_trade(ctypes.byref(compra))
    assert result == 2, f"Esperado 2 (match parcial), got {result}"
    print("test_matching_match_parcial: OK")

# ── Execução ──────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    lib = test_engine_carrega()

    test_ledger_init(lib)
    test_ledger_init_caminho_invalido(lib)

    test_validator_ordem_invalida_preco(lib)
    test_validator_ordem_invalida_quantidade(lib)
    test_validator_ordem_invalida_side(lib)
    test_validator_ordem_valida(lib)

    test_matching_sem_par_vai_para_book(lib)
    test_matching_ordem_invalida(lib)
    test_matching_match_total(lib)
    test_matching_match_parcial(lib)

    cleanup()
    print("\nTodos os testes do wrapper passaram!")
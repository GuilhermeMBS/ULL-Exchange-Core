# test_wrapper.py - Integration tests for the C engine via ctypes

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
    """Creates a populated obk_order_t."""
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

# ── Loading ───────────────────────────────────────────────────────────────────

def test_engine_loads():
    """Engine must compile and load without errors."""
    lib = load_engine()
    assert lib is not None, "Failed to load engine"
    print("test_engine_loads: OK")
    return lib

# ── Ledger ────────────────────────────────────────────────────────────────────

def test_ledger_init(lib):
    """Ledger must initialize the binary file successfully."""
    result = lib.ldg_init_ledger(LEDGER_PATH.encode())
    assert result == 0, f"Expected 0, got {result}"
    assert os.path.exists(LEDGER_PATH), "Ledger file was not created"
    print("test_ledger_init: OK")

def test_ledger_init_invalid_path(lib):
    """Ledger must return -1 for an invalid path."""
    result = lib.ldg_init_ledger(b"/invalid/path/ledger.bin")
    assert result == -1, f"Expected -1, got {result}"
    print("test_ledger_init_invalid_path: OK")

# ── Validator ─────────────────────────────────────────────────────────────────

def test_validator_invalid_price(lib):
    """Order with price <= 0 must have order_id set to -1."""
    orders = (ObkOrder * 1)()
    orders[0] = make_order(order_id=1, price=-10.0, quantity=5, side='B')
    lib.vld_validate_order(orders, 1)
    assert orders[0].order_id == ctypes.c_uint32(-1).value
    print("test_validator_invalid_price: OK")

def test_validator_invalid_quantity(lib):
    """Order with quantity <= 0 must have order_id set to -1."""
    orders = (ObkOrder * 1)()
    orders[0] = make_order(order_id=2, price=10.0, quantity=0, side='B')
    lib.vld_validate_order(orders, 1)
    assert orders[0].order_id == ctypes.c_uint32(-1).value
    print("test_validator_invalid_quantity: OK")

def test_validator_invalid_side(lib):
    """Order with side other than A or B must have order_id set to -1."""
    orders = (ObkOrder * 1)()
    orders[0] = make_order(order_id=3, price=10.0, quantity=5, side='X')
    lib.vld_validate_order(orders, 1)
    assert orders[0].order_id == ctypes.c_uint32(-1).value
    print("test_validator_invalid_side: OK")

def test_validator_valid_order(lib):
    """Valid order must not have its order_id modified."""
    orders = (ObkOrder * 1)()
    orders[0] = make_order(order_id=4, price=10.0, quantity=5, side='B')
    lib.vld_validate_order(orders, 1)
    assert orders[0].order_id == 4
    print("test_validator_valid_order: OK")

# ── Matching ──────────────────────────────────────────────────────────────────

def test_matching_no_match_goes_to_book(lib):
    """Bid order with no available ask must return 0 (queued in book)."""
    lib.mtc_reset()
    o = make_order(order_id=10, price=50.0, quantity=5, side='B')
    result = lib.mtc_make_trade(ctypes.byref(o))
    assert result == 0, f"Expected 0, got {result}"
    print("test_matching_no_match_goes_to_book: OK")

def test_matching_invalid_order(lib):
    """Invalid order (bad side) must return -1."""
    lib.mtc_reset()
    o = make_order(order_id=ctypes.c_uint32(-1).value, price=-1.0, quantity=0, side='X')
    o.is_valid = False
    result = lib.mtc_make_trade(ctypes.byref(o))
    assert result == -1, f"Expected -1, got {result}"
    print("test_matching_invalid_order: OK")

def test_matching_full_match(lib):
    """Ask and bid with same price and quantity must produce a full match (1)."""
    lib.mtc_reset()
    ask = make_order(order_id=20, price=50.0, quantity=5, side='A', timestamp=1000)
    bid = make_order(order_id=21, price=50.0, quantity=5, side='B', timestamp=1001)
    lib.mtc_make_trade(ctypes.byref(ask))
    result = lib.mtc_make_trade(ctypes.byref(bid))
    assert result == 1, f"Expected 1 (full match), got {result}"
    print("test_matching_full_match: OK")

def test_matching_partial_match(lib):
    """Bid larger than ask must produce a partial match (2)."""
    lib.mtc_reset()
    ask = make_order(order_id=30, price=50.0, quantity=3, side='A', timestamp=1000)
    bid = make_order(order_id=31, price=50.0, quantity=10, side='B', timestamp=1001)
    lib.mtc_make_trade(ctypes.byref(ask))
    result = lib.mtc_make_trade(ctypes.byref(bid))
    assert result == 2, f"Expected 2 (partial match), got {result}"
    print("test_matching_partial_match: OK")

# ── Run ───────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    lib = test_engine_loads()

    test_ledger_init(lib)
    test_ledger_init_invalid_path(lib)

    test_validator_invalid_price(lib)
    test_validator_invalid_quantity(lib)
    test_validator_invalid_side(lib)
    test_validator_valid_order(lib)

    test_matching_no_match_goes_to_book(lib)
    test_matching_invalid_order(lib)
    test_matching_full_match(lib)
    test_matching_partial_match(lib)

    cleanup()
    print("\nAll wrapper tests passed!")

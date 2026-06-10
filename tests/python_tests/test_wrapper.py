# test_wrapper.py - Integration tests for the C engine via ctypes

import ctypes
import os
import sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from engine_wrapper import load_engine, ObkOrder, MtcTransaction

LEDGER_PATH = "tests/python_tests/test_ledger_temp.bin"
MARKET_PATH = "tests/python_tests/test_market_temp.csv"


def create_test_market_csv(filename, content):
    """Helper to write short simulated input paths onto disk safely."""
    with open(filename, "w") as f:
        f.write(content)


def cleanup():
    for f in [LEDGER_PATH, MARKET_PATH]:
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
    """Ledger must initialize the binary file successfully via engine hooks."""
    mtc_handle = ctypes.c_void_p()
    assert lib.mtc_create_engine(ctypes.byref(mtc_handle)) == 0
    
    result = lib.ldg_save_ledger(LEDGER_PATH.encode(), mtc_handle, 0)
    assert result == 0, f"Expected 0, got {result}"
    assert os.path.exists(LEDGER_PATH), "Ledger file was not created"
    
    lib.mtc_clear_engine(ctypes.byref(mtc_handle))
    cleanup()
    print("test_ledger_init: OK")


def test_ledger_init_invalid_path(lib):
    """Ledger must return a validation error code for an invalid path."""
    mtc_handle = ctypes.c_void_p()
    assert lib.mtc_create_engine(ctypes.byref(mtc_handle)) == 0

    result = lib.ldg_save_ledger(b"/invalid/path/ledger.bin", mtc_handle, 0)
    assert result != 0, f"Expected error token, got {result}"
    
    lib.mtc_clear_engine(ctypes.byref(mtc_handle))
    print("test_ledger_init_invalid_path: OK")


# ── Validator ─────────────────────────────────────────────────────────────────

def test_validator_invalid_price(lib):
    """Order with price <= 0 must have order_id set to -1."""
    order = make_order(order_id=1, price=-10.0, quantity=5, side='B')
    is_valid = ctypes.c_bool(True)
    
    lib.vld_validate_order(order, ctypes.byref(is_valid))
    assert is_valid.value is False
    print("test_validator_invalid_price: OK")


def test_validator_invalid_quantity(lib):
    """Order with quantity <= 0 must have order_id set to -1."""
    order = make_order(order_id=2, price=10.0, quantity=0, side='B')
    is_valid = ctypes.c_bool(True)
    
    lib.vld_validate_order(order, ctypes.byref(is_valid))
    assert is_valid.value is False
    print("test_validator_invalid_quantity: OK")


def test_validator_invalid_side(lib):
    """Order with side other than A or B must have order_id set to -1."""
    order = make_order(order_id=3, price=10.0, quantity=5, side='X')
    is_valid = ctypes.c_bool(True)
    
    lib.vld_validate_order(order, ctypes.byref(is_valid))
    assert is_valid.value is False
    print("test_validator_invalid_side: OK")


def test_validator_valid_order(lib):
    """Valid order must not have its order_id modified."""
    order = make_order(order_id=4, price=10.0, quantity=5, side='B')
    is_valid = ctypes.c_bool(False)
    
    lib.vld_validate_order(order, ctypes.byref(is_valid))
    assert is_valid.value is True
    print("test_validator_valid_order: OK")


# ── Matching ──────────────────────────────────────────────────────────────────

def test_matching_no_match_goes_to_book(lib):
    """Bid order with no available ask must return 0 (queued in book)."""
    create_test_market_csv(
        MARKET_PATH,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,10,1,5,50.0,PETR4,B\n"
    )
    
    prs_handle = ctypes.c_void_p()
    total_orders = ctypes.c_int32(0)
    total_trades = ctypes.c_int32(0)
    mtc_handle = ctypes.c_void_p()
    
    assert lib.prs_create_orders(MARKET_PATH.encode(), ctypes.byref(prs_handle), ctypes.byref(total_orders)) == 0
    assert lib.mtc_create_engine(ctypes.byref(mtc_handle)) == 0
    
    assert lib.mtc_process_matching(mtc_handle, prs_handle, total_orders, ctypes.byref(total_trades)) == 0
    assert total_trades.value == 0
    assert lib.mtc_get_bid_count(mtc_handle) == 1
    
    lib.prs_free_buffer(prs_handle)
    lib.mtc_clear_engine(ctypes.byref(mtc_handle))
    remove_temp_files()
    print("test_matching_no_match_goes_to_book: OK")


def test_matching_invalid_order(lib):
    """Invalid order (bad side) must return -1."""
    create_test_market_csv(
        MARKET_PATH,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,15,1,5,50.0,PETR4,X\n"
    )
    
    prs_handle = ctypes.c_void_p()
    total_orders = ctypes.c_int32(0)
    total_trades = ctypes.c_int32(0)
    mtc_handle = ctypes.c_void_p()
    
    assert lib.prs_create_orders(MARKET_PATH.encode(), ctypes.byref(prs_handle), ctypes.byref(total_orders)) == 0
    assert lib.mtc_create_engine(ctypes.byref(mtc_handle)) == 0
    
    assert lib.mtc_process_matching(mtc_handle, prs_handle, total_orders, ctypes.byref(total_trades)) == 0
    assert total_trades.value == 0
    assert lib.mtc_get_bid_count(mtc_handle) == 0
    assert lib.mtc_get_ask_count(mtc_handle) == 0
    
    lib.prs_free_buffer(prs_handle)
    lib.mtc_clear_engine(ctypes.byref(mtc_handle))
    remove_temp_files()
    print("test_matching_invalid_order: OK")


def test_matching_full_match(lib):
    """Ask and bid with same price and quantity must produce a full match (1)."""
    create_test_market_csv(
        MARKET_PATH,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,20,1,5,50.0,PETR4,A\n"
        "1001,21,2,5,50.0,PETR4,B\n"
    )
    
    prs_handle = ctypes.c_void_p()
    total_orders = ctypes.c_int32(0)
    total_trades = ctypes.c_int32(0)
    mtc_handle = ctypes.c_void_p()
    
    assert lib.prs_create_orders(MARKET_PATH.encode(), ctypes.byref(prs_handle), ctypes.byref(total_orders)) == 0
    assert lib.mtc_create_engine(ctypes.byref(mtc_handle)) == 0
    
    assert lib.mtc_process_matching(mtc_handle, prs_handle, total_orders, ctypes.byref(total_trades)) == 0
    assert total_trades.value == 1
    
    assert lib.ldg_save_ledger(LEDGER_PATH.encode(), mtc_handle, total_trades) == 0
    assert os.path.exists(LEDGER_PATH)
    
    lib.prs_free_buffer(prs_handle)
    lib.mtc_clear_engine(ctypes.byref(mtc_handle))
    remove_temp_files()
    print("test_matching_full_match: OK")


def test_matching_partial_match(lib):
    """Bid larger than ask must produce a partial match (2)."""
    create_test_market_csv(
        MARKET_PATH,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1000,30,1,3,50.0,PETR4,A\n"
        "1001,31,2,10,50.0,PETR4,B\n"
    )
    
    prs_handle = ctypes.c_void_p()
    total_orders = ctypes.c_int32(0)
    total_trades = ctypes.c_int32(0)
    mtc_handle = ctypes.c_void_p()
    
    assert lib.prs_create_orders(MARKET_PATH.encode(), ctypes.byref(prs_handle), ctypes.byref(total_orders)) == 0
    assert lib.mtc_create_engine(ctypes.byref(mtc_handle)) == 0
    
    assert lib.mtc_process_matching(mtc_handle, prs_handle, total_orders, ctypes.byref(total_trades)) == 0
    assert total_trades.value == 1
    assert lib.mtc_get_bid_count(mtc_handle) == 1
    
    lib.prs_free_buffer(prs_handle)
    lib.mtc_clear_engine(ctypes.byref(mtc_handle))
    remove_temp_files()
    print("test_matching_partial_match: OK")


def remove_temp_files():
    if os.path.exists(MARKET_PATH):
        os.remove(MARKET_PATH)
    if os.path.exists(LEDGER_PATH):
        os.remove(LEDGER_PATH)


# ── Run ───────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    print("=" * 60)
    print("  ENGINE WRAPPER TESTS")
    print("=" * 60)

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
    print("=" * 60)
    print("\nALL WRAPPER TESTS PASSED!")
    print("=" * 60)

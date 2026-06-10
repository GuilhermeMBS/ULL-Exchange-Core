# engine_wrapper.py - Compiles the C source files and exposes functions via ctypes

import ctypes
import subprocess

# ── Structures ────────────────────────────────────────────────────────────────

class ObkOrder(ctypes.Structure):
    _fields_ = [
        ("timestamp", ctypes.c_uint32),
        ("order_id",  ctypes.c_uint32),
        ("client_id", ctypes.c_uint32),
        ("quantity",  ctypes.c_uint32),
        ("price",     ctypes.c_double),
        ("symbol",    ctypes.c_char * 8),
        ("side",      ctypes.c_char),
        ("is_valid",  ctypes.c_bool),
    ]

class MtcTransaction(ctypes.Structure):
    _fields_ = [
        ("timestamp",      ctypes.c_uint32),
        ("trade_id",       ctypes.c_int32),
        ("buy_order_id",   ctypes.c_int32),
        ("sell_order_id",  ctypes.c_int32),
        ("buy_client_id",  ctypes.c_int32),
        ("sell_client_id", ctypes.c_int32),
        ("price",          ctypes.c_double),
        ("quantity",       ctypes.c_int32),
    ]

# ── Loading ───────────────────────────────────────────────────────────────────

def load_engine():
    """Compiles C source files and loads the shared library."""

    result = subprocess.run([
        "gcc", "-shared", "-fPIC",
        "core/src/book.c",
        "core/src/retcodes.c",
        "core/src/matching.c",
        "core/src/parserlib.c",
        "core/src/ledger.c",
        "core/src/validator.c",
        "-Icore/include",
        "-o", "engine.so"
    ], capture_output=True, text=True)

    if result.returncode != 0:
        print("Error compiling engine:")
        print(result.stderr)
        return None

    lib = ctypes.CDLL("./engine.so")

    # Mapped according to the new opaque pointer TAD interfaces
    lib.prs_create_orders.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_int32)]
    lib.prs_create_orders.restype  = ctypes.c_int32

    lib.prs_get_order_by_index.argtypes = [ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(ObkOrder)]
    lib.prs_get_order_by_index.restype  = ctypes.c_int32

    lib.prs_free_buffer.argtypes = [ctypes.c_void_p]
    lib.prs_free_buffer.restype  = ctypes.c_int32

    lib.mtc_create_engine.argtypes = [ctypes.POINTER(ctypes.c_void_p)]
    lib.mtc_create_engine.restype  = ctypes.c_int32

    lib.mtc_process_matching.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(ctypes.c_int32)]
    lib.mtc_process_matching.restype  = ctypes.c_int32

    lib.mtc_get_trade_by_index.argtypes = [ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(MtcTransaction)]
    lib.mtc_get_trade_by_index.restype  = ctypes.c_int32

    lib.mtc_clear_engine.argtypes = [ctypes.POINTER(ctypes.c_void_p)]
    lib.mtc_clear_engine.restype  = ctypes.c_int32

    lib.ldg_save_ledger.argtypes = [ctypes.c_char_p, ctypes.c_void_p, ctypes.c_int32]
    lib.ldg_save_ledger.restype  = ctypes.c_int32

    lib.vld_validate_order.argtypes = [ObkOrder, ctypes.POINTER(ctypes.c_bool)]
    lib.vld_validate_order.restype  = ctypes.c_int32

    return lib

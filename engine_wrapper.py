# Unir os arquivos C em .dll

# engine_wrapper.py - Compila os arquivos C e expõe as funções via ctypes

import ctypes
import subprocess

# ── Estruturas ────────────────────────────────────────────────────────────────

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

# ── Carregamento ──────────────────────────────────────────────────────────────

def load_engine():
    """Compila os arquivos C e carrega a shared library."""

    result = subprocess.run([
        "gcc", "-shared", "-fPIC",
        "core/src/book.c",
        "core/src/errorlib.c",
        "core/src/matching.c",
        "core/src/parser.c",
        "core/src/ledger.c",
        "core/src/validator.c",
        "-Icore/include",
        "-o", "engine.so"
    ], capture_output=True, text=True)

    if result.returncode != 0:
        print("Erro ao compilar engine:")
        print(result.stderr)
        return None

    lib = ctypes.CDLL("./engine.so")

    lib.prs_create_orders.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_int32)]
    lib.prs_create_orders.restype  = ctypes.POINTER(ObkOrder)

    lib.prs_free_buffer.argtypes = [ctypes.POINTER(ObkOrder)]
    lib.prs_free_buffer.restype  = ctypes.c_int32

    lib.mtc_make_trade.argtypes = [ctypes.POINTER(ObkOrder)]
    lib.mtc_make_trade.restype  = ctypes.c_int32

    lib.ldg_init_ledger.argtypes = [ctypes.c_char_p]
    lib.ldg_init_ledger.restype  = ctypes.c_int32

    lib.ldg_register_trade.argtypes = [ctypes.POINTER(MtcTransaction)]
    lib.ldg_register_trade.restype  = ctypes.c_int32

    return lib
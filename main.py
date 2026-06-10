import sys
from scripts.generate_market import generate_market
from engine_wrapper import load_engine
import ctypes

MARKET_PATH  = "data/market.csv"
LEDGER_PATH  = "data/ledger.bin"
NUM_ORDERS   = 1000
INVALID_RATE = 0.05


def start_engine():
    """Orchestrates the full exchange engine flow."""

    print("\n============ RUNNING FULL APPLICATION ===========\n")

    # 1. Generate market CSV
    result = generate_market(MARKET_PATH, num_orders=NUM_ORDERS, failures=INVALID_RATE)
    if result != 0:
        print(f"Error generating market: {result}")
        return result

    # Load C engine
    lib = load_engine()
    if lib is None:
        print("Error loading engine.")
        return -1

    # 2. Main takes the CSV and invokes the Parser with opaque handle pointers
    prs_handle = ctypes.c_void_p()
    total_count = ctypes.c_int32(0)
    
    # prs_create_orders reads line by line, clones to validator and populates the hidden buffer (Steps 3, 4, 5)
    result = lib.prs_create_orders(MARKET_PATH.encode(), ctypes.byref(prs_handle), ctypes.byref(total_count))
    if result != 0 or not prs_handle.value:
        print("Error parsing orders into C container.")
        return -1
    print(f"{total_count.value} orders processed into secure hidden buffer structure.")

    # 6. Main instantiates the Matching Engine opaque handle descriptor object
    mtc_handle = ctypes.c_void_p()
    result = lib.mtc_create_engine(ctypes.byref(mtc_handle))
    if result != 0 or not mtc_handle.value:
        print("Error initializing matching engine core.")
        lib.prs_free_buffer(prs_handle)
        return -1

    # Intersubs Module Cascade: Matching engine loops sequentially via parser opaque indexers (Steps 7, 8, 9)
    total_trades = ctypes.c_int32(0)
    result = lib.mtc_process_matching(mtc_handle, prs_handle, total_count, ctypes.byref(total_trades))
    if result != 0:
        print("Error processing market order matching.")
        lib.prs_free_buffer(prs_handle)
        lib.mtc_clear_engine(ctypes.byref(mtc_handle))
        return -1
    print(f"Matching complete. {total_trades.value} trades safely recorded into hidden engine structure.")

    # 10. Main invokes the ledger module to dump memory buffers safely into disco disk binaries
    # O Ledger scans matching buffer using opaque data extraction getters (Step 11)
    result = lib.ldg_save_ledger(LEDGER_PATH.encode(), mtc_handle, total_trades)
    if result != 0:
        print(f"Error persisting ledger database: {result}")
        lib.prs_free_buffer(prs_handle)
        lib.mtc_clear_engine(ctypes.byref(mtc_handle))
        return -1
    print(f"Ledger database successfully saved to: {LEDGER_PATH}")

    # 12. Execution clean up lifecycle termination releases heap resource tracks safely
    lib.prs_free_buffer(prs_handle)
    lib.mtc_clear_engine(ctypes.byref(mtc_handle))

    print("\n============ EXECUTION COMPLETED SUCCESSFULLY ===========")
    return 0


if __name__ == "__main__":
    result = start_engine()
    print(f"\nProgram exited with code: {result}")
    sys.exit(result)

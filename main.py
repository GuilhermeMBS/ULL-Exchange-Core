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


    # Generate market CSV
    result = generate_market(MARKET_PATH, num_orders=NUM_ORDERS, failures=INVALID_RATE)
    if result != 0:
        print(f"Error generating market: {result}")
        return result

    # Load C engine
    lib = load_engine()
    if lib is None:
        print("Error loading engine.")
        return -1

    # Initialize ledger
    result = lib.ldg_init_ledger(LEDGER_PATH.encode())
    if result != 0:
        print(f"Error initializing ledger: {result}")
        return result

    # Parse CSV into C orders
    total_count = ctypes.c_int32(0)
    orders = lib.prs_create_orders(MARKET_PATH.encode(), ctypes.byref(total_count))
    if not orders:
        print("Error parsing orders.")
        return -1
    print(f"{total_count.value} orders loaded.")

    # Process each order through the matching engine
    trades = 0
    for i in range(total_count.value):
        result = lib.mtc_make_trade(ctypes.byref(orders[i]))
        if result == 1 or result == 2:  # Full match or partial match
            trades += 1

    print(f"Matching complete. {trades} trades executed.")

    # Free order buffer
    lib.prs_free_buffer(orders)

    print(f"Ledger saved to: {LEDGER_PATH}")
    print("\n============ EXECUTION COMPLETED SUCCESSFULLY ===========")
    
    return 0


if __name__ == "__main__":
    result = start_engine()
    print(f"\nProgram exited with code: {result}")
    sys.exit(result)

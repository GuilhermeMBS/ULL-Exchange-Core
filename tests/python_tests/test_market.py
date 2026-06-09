import os
import sys
import csv
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from scripts.generate_market import generate_market

OUTPUT = "tests/python_tests/test_output.csv"

def cleanup():
    if os.path.exists(OUTPUT):
        os.remove(OUTPUT)

def test_returns_zero_on_success():
    result = generate_market(OUTPUT, num_orders=10, failures=0.0)
    assert result == 0, f"Expected 0, got {result}"
    cleanup()
    print("test_returns_zero_on_success: OK")

def test_returns_error_on_invalid_path():
    result = generate_market("/invalid/path/x.csv", 10, 0.0)
    assert result == -4, f"Expected -4, got {result}"
    print("test_returns_error_on_invalid_path: OK")

def test_file_is_created():
    generate_market(OUTPUT, num_orders=10, failures=0.0)
    assert os.path.exists(OUTPUT), "File was not created"
    cleanup()
    print("test_file_is_created: OK")

def test_100_mixed_orders():
    result = generate_market(OUTPUT, num_orders=100, failures=0.1)
    assert result == 0, f"Expected 0, got {result}"
    cleanup()
    print("test_100_mixed_orders: OK")

def test_correct_line_count():
    generate_market(OUTPUT, num_orders=100, failures=0.1)
    with open(OUTPUT) as f:
        lines = list(csv.reader(f))
    assert len(lines) == 101, f"Expected 101 lines, got {len(lines)}"
    cleanup()
    print("test_correct_line_count: OK")

def test_timestamps_are_ascending():
    generate_market(OUTPUT, num_orders=100, failures=0.0)
    with open(OUTPUT) as f:
        reader = csv.DictReader(f)
        timestamps = [int(row['timestamp']) for row in reader]
    assert timestamps == sorted(timestamps), "Timestamps out of order"
    cleanup()
    print("test_timestamps_are_ascending: OK")

def test_side_only_A_or_B():
    generate_market(OUTPUT, num_orders=100, failures=0.0)
    with open(OUTPUT) as f:
        reader = csv.DictReader(f)
        for row in reader:
            assert row['side'] in ['A', 'B'], f"Invalid side: {row['side']}"
    cleanup()
    print("test_side_only_A_or_B: OK")

def test_invalid_orders_have_bad_price_or_qty():
    generate_market(OUTPUT, num_orders=100, failures=1.0)
    with open(OUTPUT) as f:
        reader = csv.DictReader(f)
        for row in reader:
            price = float(row['price'])
            qty   = float(row['quantity'])
            assert price <= 0 or qty <= 0, "Order should be invalid"
    cleanup()
    print("test_invalid_orders_have_bad_price_or_qty: OK")

def test_valid_orders_have_positive_price_and_qty():
    generate_market(OUTPUT, num_orders=100, failures=0.0)
    with open(OUTPUT) as f:
        reader = csv.DictReader(f)
        for row in reader:
            assert float(row['price']) > 0, "Price should be positive"
            assert float(row['quantity']) > 0, "Quantity should be positive"
    cleanup()
    print("test_valid_orders_have_positive_price_and_qty: OK")

def test_correct_header():
    generate_market(OUTPUT, num_orders=10, failures=0.0)
    with open(OUTPUT) as f:
        header = csv.reader(f).__next__()
    expected = ['timestamp', 'order_id', 'client_id', 'quantity', 'price', 'symbol', 'side']
    assert header == expected, f"Wrong header: {header}"
    cleanup()
    print("test_correct_header: OK")

if __name__ == "__main__":
    print("=" * 60)
    print("  MARKET GENERATION TEST")
    print("=" * 60)

    test_returns_zero_on_success()
    test_returns_error_on_invalid_path()
    test_file_is_created()
    test_100_mixed_orders()
    test_correct_line_count()
    test_timestamps_are_ascending()
    test_side_only_A_or_B()
    test_invalid_orders_have_bad_price_or_qty()
    test_valid_orders_have_positive_price_and_qty()
    test_correct_header()

    print("=" * 60)
    print("\nALL MARKET TESTS PASSED!")
    print("=" * 60)

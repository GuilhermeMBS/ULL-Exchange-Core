# Generates a CSV of simulated market orders (valid and invalid) for the engine.

import csv
import random
import sys

SYMBOLS = ["PETR4", "VALE3", "ITUB4", "BBDC4", "ABEV3"]

def generate_market(filename, num_orders, failures):
    """
    Generates a CSV file with simulated market orders.

    Output format: timestamp,order_id,client_id,quantity,price,symbol,side
      - side: 'A' (Ask/Sell) or 'B' (Bid/Buy)
      - timestamp: incrementing integer (Unix-like)

    Args:
        filename   (str):   Path to the CSV file to be created
        num_orders (int):   Total number of orders to generate
        failures   (float): Proportion of invalid orders (0.0 to 1.0)

    Returns:
        int: 0 on success, -4 on file creation error
    """
    try:
        with open(filename, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['timestamp', 'order_id', 'client_id', 'quantity', 'price', 'symbol', 'side'])

            base_time = 1748000000

            for i in range(num_orders):
                timestamp = base_time + i
                order_id  = i + 1
                client_id = random.randint(1, 100)
                symbol    = random.choice(SYMBOLS)
                side      = random.choice(['A', 'B'])

                if random.random() < failures:
                    price = random.choice([-1.0, 0.0])
                    qty   = random.choice([0, -5])
                else:
                    price = round(random.uniform(10.0, 200.0), 2)
                    qty   = random.randint(1, 100)

                writer.writerow([timestamp, order_id, client_id, qty, price, symbol, side])

        return 0

    except Exception as e:
        # print(f"Error generating market: {e}", file=sys.stderr)
        return -4

if __name__ == "__main__":
    result = generate_market("data/market.csv", num_orders=1000, failures=0.1)
    print(f"generate_market returned: {result}")

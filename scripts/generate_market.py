# Gerar um CSV de 1000 ordens variadas (válidas e inválidas) e aleatórias

import csv
import random
import sys
from datetime import datetime, timedelta

def generateMarket(filename, num_orders, failures):
    try:
        with open(filename, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['id', 'timestamp', 'side', 'price', 'quantity'])

            base_time = datetime(2026, 1, 1, 9, 0, 0)

            for i in range(num_orders):
                timestamp = base_time + timedelta(seconds=i)

                if random.random() < failures:
                    price = random.choice([-1, 0])
                    qty = random.choice([-5, 0])
                else:
                    price = round(random.uniform(10.0, 200.0), 2)
                    qty = random.randint(1, 100)

                side = random.choice(['C', 'V'])
                writer.writerow([i+1, timestamp.isoformat(), side, price, qty])

        return 0

    except Exception as e:
        print(f"Erro ao gerar mercado: {e}", file=sys.stderr) 
        return -4

if __name__ == "__main__":
    result = generateMarket("data/market.csv", num_orders=1000, failures=0.1) #vai ter algumas inválidas
    print(f"generateMarket retornou: {result}")
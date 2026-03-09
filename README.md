# Ultra-Low Latency Matching Engine (LOB)

A modular, high-performance **Matching Engine** written in **pure C** with a **Python** orchestration layer. This project simulates a financial exchange's core logic, managing a **Limit Order Book (LOB)** with strict price-time priority.

## 🚀 Key Features
- **Ultra-Low Latency:** Designed with C for minimal overhead and direct memory management.
- **Strictly Modular:** Architecture based on independent modules (Parser, Validator, Engine, Book, Ledger) for high maintainability and testability.
- **Price-Time Priority:** Implements standard exchange matching rules.
- **Zero-Classes Policy:** Built using procedural C and Python scripts (no OOP) to focus on raw performance and data locality.
- **Persistence:** High-speed binary logging for trade auditing and statistics.

## 🏗️ Architecture
The system is divided into five core functional modules:
1. **Parser:** Interface between Python and C; data sanitization.
2. **Validator:** Risk management and business rule enforcement.
3. **Order Book (LOB):** Efficient data structures (Heaps/Linked Lists) for order storage.
4. **Matching Engine:** The core logic for crossing orders and partial fills.
5. **Ledger & Stats:** Persistence layer and real-time performance metrics.



## 🛠️ Tech Stack
- **Engine:** C (C11 standard)
- **Orchestration/Testing:** Python 3.x
- **Integration:** Ctypes (Foreign Function Interface)

## 📊 Performance Focus
This engine is designed to minimize cache misses by ensuring **Data Locality**. By managing memory manually in C, we avoid the latencies typically introduced by Garbage Collectors (GC) or heavy abstractions found in C++ or Java.

## 🧪 Testing
Each module includes a dedicated **Testing Module** ensuring 100% logic coverage, as required for mission-critical financial software.

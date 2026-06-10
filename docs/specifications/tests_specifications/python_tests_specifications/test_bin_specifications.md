# Test Documentation — Binary Ledger Validator (`test_bin.py`)

This document describes the testing logic used by the automated system-level post-execution verifier framework. The module works downstream, parsing the generated `market.csv` input logs and reading back the resulting encrypted `ledger.bin` output files to guarantee absolute trade record precision.

## Implemented Validation Steps

### 1. Ingestion Integrity Map
* **Method:** `load_orders`
* **Objective:** Parse the entire raw generation history data, storing entries in an internal dictionary indexed by `order_id` for fast cross-referencing.

### 2. Low-Level Deserialization Check
* **Method:** `load_trades`
* **Objective:** Open the final `ledger.bin` database file in read-binary mode (`'rb'`), slice the continuous stream into blocks matching the exact `ctypes.sizeof(MtcTransaction)` configuration, and convert them back into structured data models using `from_buffer_copy`.

### 3. Structural Constraint Verification (System Rules)
* **Method:** `verify` (Rules 1, 2 & 3)
* **Objective:** Enforce technical baseline rules on every deserialized block.
* **Verification:** Asserts that every logged `trade_id` is unique and strictly positive. Asserts that execution price and matched volumes are strictly greater than zero. Validates that references do not point to unresolved entries (`-1`).

### 4. Cross-Module Data Parity Verification
* **Method:** `verify` (Rules 4 & 5)
* **Objective:** Match the binary database parameters against the input logs.
* **Verification:** Extracts the matched `buy_order_id` and `sell_order_id` from the binary block and verifies they exist in the CSV dictionary map. Asserts that the buyer structure was originally a Bid ('B') and the seller structure was originally an Ask ('A').

### 5. Financial Boundary Price Check
* **Method:** `verify` (Rule 6)
* **Objective:** Enforce arbitrage and trading execution limits (`Bid >= Execution Price >= Ask`).
* **Verification:** Asserts that the final cleared price never exceeds the buyer's maximum price limit and never falls below the seller's minimum price threshold.

### 6. Volume Deficit Allocation Check
* **Method:** `verify` (Rule 7)
* **Objective:** Ensure the clearing house never executes a matched size that exceeds the smaller available volume of the two counterparties.
* **Verification:** Logs a warning if the matched transaction size exceeds either the original buyer's request or the seller's allocation limit.

### 7. Self-Trading Guard Anomalies Tracking
* **Method:** `verify` (Rule 8)
* **Objective:** Audit the output array against internal wash-trading or self-matching anomalies.
* **Verification:** Flags a system warning if `buy_client_id` matches `sell_client_id`, detecting accounts trading with themselves.

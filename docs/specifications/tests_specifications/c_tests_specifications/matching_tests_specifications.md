# Test Documentation — Matching Engine Module (`matching.c`)

This document describes the validation suite developed to evaluate the Price/Time automated clearing house execution flow (`matching.c`). The engine uses an Abstract Data Type (ADT) pattern that handles matches sequentially via opaque parser handles.

## Implemented Test Cases

### 1. Null Engine Context Rejection
* **Target Function:** `mtc_process_matching`
* **Objective:** Ensure that invocations containing null opaque references are caught safely.
* **Verification:** Passes null values into both the engine and parser parameter properties, confirming that it yields an `ERR_ORD` return token.

### 2. Invalid Field Routing Skipping
* **Target Function:** `mtc_process_matching`
* **Objective:** Verify that data blocks marked as corrupt are ignored and skipped entirely without altering the order book state.
* **Verification:** Injects a data row containing a side attribute mapped as `'X'`. Runs the clearing function and uses introspection getters to assert that both the buy and sell sides remain completely untouched.

### 3. Queueing on Empty Opposite Side
* **Target Function:** `mtc_process_matching`
* **Objective:** Validate that incoming entries that find no immediate opposite match are saved safely into the order book as resting orders.
* **Verification:** Processes a buy entry ('B') and verifies that the `mtc_get_bid_count` tracking increases to `1` while the ask counter stays at `0`. Clears the context and runs the inverse scenario for a sell order ('A').

### 4. Price Cross Boundaries Miss
* **Target Function:** `mtc_process_matching`
* **Objective:** Ensure that orders whose prices do not cross (e.g., a Bid price lower than the best available Ask) do not trigger an incorrect clearing transaction.
* **Verification:** Registers an active Ask at `$100.0` and inputs an incoming Bid at `$90.0`. Asserts that both items remain queued passively in memory with a trade execution count of zero.

### 5. Full Parity Execution Match
* **Target Function:** `mtc_process_matching` and `mtc_get_trade_by_index`
* **Objective:** Validate a complete match scenario where an incoming order matches a resting order in both price and size.
* **Verification:** Loads an Ask for 200 units at `$100.0` followed by a Bid for 200 units at `$100.0`. Asserts that the matching engine clears both positions, resets book depths to zero, and logs 1 trade record containing the crossed IDs.

### 6. Partial Match — Bid Deficit Case
* **Target Function:** `mtc_process_matching` and `mtc_get_best_ask`
* **Objective:** Validate partial execution behavior when an incoming Buy volume is smaller than the available resting Sell order size.
* **Verification:** Inputs a resting Ask for 500 units and an incoming Bid for 200 units. Asserts that a transaction for 200 units is generated and verifies that the remaining Ask stays at the top of the book with its quantity reduced to exactly 300 units.

### 7. Partial Match — Ask Deficit Case
* **Target Function:** `mtc_process_matching` and `mtc_get_best_bid`
* **Objective:** Validate partial execution behavior when an incoming Sell volume is smaller than the available resting Buy order size.
* **Verification:** Records a resting Bid for 500 units and logs an incoming Ask for 200 units. Asserts the partial match and verifies that the remaining Buy order stays at the top of the book with its volume reduced to 300 units.

### 8. Recursive Multilevel Execution Sorting
* **Target Function:** `mtc_process_matching`
* **Objective:** Evaluate the engine's recursive capability to walk down multiple priority levels when a single large-volume order sweeps multiple smaller resting orders.
* **Verification:** Accumulates three individual resting Ask entries for 100 units each (300 units total depth). Injects a single matching Bid for 300 units and asserts that the engine triggers a cascading clearing sequence, emptying the book and logging 3 distinct execution records.

### 9. Resumption Indexing and Double Insertion Guard
* **Target Function:** `mtc_process_matching`
* **Objective:** Ensure that consecutive calls to the matching function do not cause previously parsed entries to be processed again.
* **Verification:** Runs the matching loop with a parser length constraint of `1`, and then triggers a second pass for the total length. Asserts that the engine saves its parsing position internally (`last_processed_idx`), ensuring that order volumes are processed accurately without duplication.

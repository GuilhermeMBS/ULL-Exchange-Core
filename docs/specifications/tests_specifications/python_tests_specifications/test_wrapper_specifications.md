# Test Documentation — C Engine Wrapper Integration (`test_wrapper.py`)

This document describes the high-level integration testing suite developed to validate the interaction between Python and the underlying native C shared library (`engine.so`) via `ctypes`. It guarantees that Abstract Data Type (ADT) parameters and binary memory layout fields are perfectly aligned.

## Implemented Test Cases

### 1. Shareable Object Ingestion and Compilation
* **Target Function:** `load_engine`
* **Objective:** Validate that the runtime subsystem compiles the C source tree smoothly via `subprocess` handles and yields a valid, loadable binary link.
* **Verification:** Asserts that the returned dynamic context handle object reference is not null (`None`).

### 2. Opaque Ledger File Allocation
* **Target Function:** `ldg_save_ledger`
* **Objective:** Validate that the binary persistence component successfully handles zero-record contexts when bound to a clean matching engine instance pointer.
* **Verification:** Instantiates a matching handle (`c_void_p`), passes it to `ldg_save_ledger` with zero entries, and checks that an empty output binary database is initialized on disk.

### 3. File Directory Exception Mapping
* **Target Function:** `ldg_save_ledger`
* **Objective:** Certify that the C backend intercepts system writing restriction errors safely, returning a non-zero fault state to Python without crashing the script.
* **Verification:** Attempts to force a binary dump into an protected OS directory path and asserts a non-zero error token response.

### 4. Validator Interface — Price Boundary Violation
* **Target Function:** `vld_validate_order`
* **Objective:** Ensure that the C individual validation layer accurately catches invalid prices.
* **Verification:** Injects a python `ObkOrder` struct with `price = -10.0` into the shared memory segment, executes `vld_validate_order`, and asserts that the returned verification reference flag falls back to `False`.

### 5. Validator Interface — Quantity Volume Sinking
* **Target Function:** `vld_validate_order`
* **Objective:** Ensure that volume sizing rule violations are detected.
* **Verification:** Packs a structure where `quantity = 0` and asserts that the reference parameter is toggled to `False` by the dynamic link routine.

### 6. Validator Interface — Invalid Side Character
* **Target Function:** `vld_validate_order`
* **Objective:** Ensure that rogue side descriptor tags are caught.
* **Verification:** Sends an entry marked as `side = 'X'`, asserting that the validation callback updates the target reference flag to `False`.

### 7. Validator Interface — Production Parity Preserved
* **Target Function:** `vld_validate_order`
* **Objective:** Confirm that a completely valid record leaves the structure flag verified.
* **Verification:** Supplies a flawless `ObkOrder` asset structure and asserts that the returned validation boolean stays firmly flagged as `True`.

### 8. Passive Book Queueing Execution Trace
* **Target Function:** `mtc_process_matching`
* **Objective:** Validate the cascaded entry routine where an incoming Bid order finding an empty book is placed passively into the book depth stack.
* **Verification:** Records an isolated temporary CSV buy row, runs the parser, processes it through the matching engine via opaque reference casting, and asserts that `mtc_get_bid_count` returns `1` with total executions remaining at `0`.

### 9. Rogue Order Cascade Resilience
* **Target Function:** `mtc_process_matching`
* **Objective:** Confirm that the full multi-module pipeline automatically skips invalid rows without corrupting the order book state.
* **Verification:** Writes a market line containing an illegal character side (`'X'`), chains the parser and engine execution runs, and asserts that all transaction metrics and depth trackers remain safely at zero.

### 10. Automated Clearing Full Parity Match
* **Target Function:** `mtc_process_matching` and `ldg_save_ledger`
* **Objective:** Validate that perfectly matched cross orders clear the book and log tracking variables accurately.
* **Verification:** Feeds two entries matching in both price and size (`A` followed by `B`). Asserts that the matching engine executes the trade, clears the depth structures, and successfully dumps a valid transaction stream to disk via `ldg_save_ledger`.

### 11. Partial Execution Stack Tracker
* **Target Function:** `mtc_process_matching`
* **Objective:** Certify that multi-module partial matches correctly clear matching volumes and keep the remaining balance alive in the book.
* **Verification:** Injects a short Ask for 3 units followed by a larger Bid for 10 units. Asserts the partial transaction is captured and checks that the remaining `7` units of the Bid are safely queued in the book depths (`mtc_get_bid_count == 1`).

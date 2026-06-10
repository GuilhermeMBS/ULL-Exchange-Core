# Test Documentation — Market Generation Module (`test_market.py`)

This document describes the validation suite developed to certify the correctness, boundary limits, and data distribution integrity of the algorithmic market generator framework (`generate_market.py`).

## Implemented Test Cases

### 1. Standard Success Code Return
* **Target Function:** `generate_market`
* **Objective:** Ensure the script completes execution normally and returns a neutral code state token (`0`) when given flawless runtime variables.
* **Verification:** Calls the generation loop requesting 10 clean orders, checks the returned integer, and cleans up the generated file.

### 2. File Path Violation Exception Trapping
* **Target Function:** `generate_market`
* **Objective:** Validate that the framework catches OS file access faults elegantly and returns a failure code (`-4`) instead of raising unhandled tracebacks.
* **Verification:** Forces a write path into an impossible root destination hierarchy (`"/invalid/path/x.csv"`) and asserts that the resulting exception handler yields the correct return value.

### 3. File Creation and Lifecycle Check
* **Target Function:** `generate_market`
* **Objective:** Certify that the physical file descriptor is created on disk.
* **Verification:** Triggers generation and performs a physical checking routine via `os.path.exists()` to assert that the file has been recorded.

### 4. Mixed Order Allocation Logic
* **Target Function:** `generate_market`
* **Objective:** Validate that the engine distributes a custom error failure rate accurately within a larger continuous batch generation.
* **Verification:** Requests 100 entries with a configured `10%` failure rate. Parses the result line by line to verify structural parity.

### 5. Line Counting Constraints
* **Target Function:** `generate_market`
* **Objective:** Guarantee that the generated table size perfectly mirrors the client's payload requests.
* **Verification:** Demands a precise batch of `100` orders and checks that the final file contains exactly `101` lines (incorporating the metadata header row).

### 6. Ascending Chronological Timestamps
* **Target Function:** `generate_market`
* **Objective:** Enforce the data simulation constraint that mandates monotonic, forward-moving timeline increments.
* **Verification:** Scans the stream sequentially, comparing entry `[n]` against entry `[n-1]` to assert that timestamps are strictly ascending.

### 7. Side Domain Categorization Boundaries
* **Target Function:** `generate_market`
* **Objective:** Enforce that order sides are strictly categorized under authorized domain keys.
* **Verification:** Loops across every generated data line and asserts that the side character attribute resolves exclusively to either `'A'` (Ask) or `'B'` (Bid).

### 8. Corrupt Line Attribute Sinking
* **Target Function:** `generate_market`
* **Objective:** Ensure that entries generated as part of the failure rate metric are physically corrupted via a price or quantity sinking mechanism (`<= 0`).
* **Verification:** Inspects records with `.is_valid = false` properties and asserts that their price or quantity evaluates to an unacceptable state.

### 9. Production Entry Asset Parameters
* **Target Function:** `generate_market`
* **Objective:** Verify that clean production entries possess strictly positive asset values (`> 0`).
* **Verification:** Filters all active records and asserts that both price values and batch lots are strictly positive.

### 10. Table Column Header Parity
* **Target Function:** `generate_market`
* **Objective:** Validate that the CSV column layout layout perfectly maps onto the target schema required by the C parser module.
* **Verification:** Pulls the first index row of the file descriptor stream and asserts an exact list match against the expected signature fields: `['timestamp', 'order_id', 'client_id', 'quantity', 'price', 'symbol', 'side']`.

# Test Documentation — Parser Module (`parser.c`)

This document describes the unit testing suite developed to validate the ingestion, string tokenization, and buffer storage pipeline of the market CSV engine (`parser.c`). The component reads line-by-line file descriptors from disk, unloads attributes into arrays, and passes entries to the validation tier.

## Implemented Test Cases

### 1. NULL Arguments Handling
* **Target Function:** `prs_create_orders`
* **Objective:** Ensure the public API interface is robust against null pointers passed as argument parameters.
* **Verification:** Invokes order creation with invalid or null paths, handles, and counters. Asserts that the component handles the issue gracefully by returning the error token `ERR_ORD` instead of crashing.

### 2. Nonexistent File Rejection
* **Target Function:** `prs_create_orders`
* **Objective:** Validate subsystem behavior when a requested file path does not exist on disk.
* **Verification:** Passes a dummy filename (`"file_that_does_not_exist.csv"`) and asserts that the engine aborts cleanly by returning `ERR_ORD` and keeping the output pointer reference tied to `NULL`.

### 3. Empty File Rejection
* **Target Function:** `prs_create_orders`
* **Objective:** Validate the rejection of empty data files containing zero bytes (no headers, no lines).
* **Verification:** Writes a blank file called `"empty.csv"`, passes it to the parser, and asserts that the structural compilation returns `ERR_ORD`.

### 4. Header-Only File Rejection
* **Target Function:** `prs_create_orders`
* **Objective:** Verify that market records containing only table field names with no underlying data lines are rejected.
* **Verification:** Generates a temporary `"header.csv"` file with column metadata and asserts the error return `ERR_ORD`.

### 5. Single Valid Order Ingestion
* **Target Function:** `prs_create_orders` and `prs_get_order_by_index`
* **Objective:** Validate field mapping correctness (Timestamp, ID, Client, Qty, Price, Symbol, Side) from text lines into structural attributes.
* **Verification:** Processes a well-formed CSV file string line, invokes the parser, and fetches index `0` to execute direct property matching assertions against the result `obk_order_t` object attributes.

### 6. Multiple Valid Orders and Boundary Retrieval
* **Target Function:** `prs_create_orders` and `prs_get_order_by_index`
* **Objective:** Validate continuous multi-line batch loading, index sequence tracking preservation, and boundary constraint checks.
* **Verification:** Loads a test target file containing three valid order lines, asserts that the loaded output counter reports exactly `3`, and verifies that trying to request an out-of-bounds slot index (such as `3`) safely returns the error token `ERR_ORD`.

### 7. Malformed Line Isolation
* **Target Function:** `prs_create_orders`
* **Objective:** Ensure that an incomplete or corruped text row is isolated within its internal slot buffer segment rather than crashing the batch pipeline.
* **Verification:** Injects a line missing required trailing parameters (`"1000,1,10"`), processes it, and asserts that the resulting index entry is safely normalized with `.is_valid = false` and `.order_id = -1`.

### 8. Validator Integration Automatic Chain
* **Target Function:** `prs_create_orders`
* **Objective:** Verify the inline modular chain integration that triggers validation business rules on parsed records automatically.
* **Verification:** Feeds an entry that is structurally valid but breaks business parameters (e.g., `quantity = 0`). Asserts that the inline business logic automatically intercepts this row, resulting in the item being saved with `.is_valid = false` and `.order_id = -1` in the final buffer.

### 9. Buffer Deallocation Lifecycle
* **Target Function:** `prs_free_buffer`
* **Objective:** Validate safe memory deallocation routines at the end of the data lifespan.
* **Verification:** Sends null and valid pointers to the clean-up engine, asserting that passing `prs_free_buffer(NULL)` returns `ERR_ORD` under controlled parameters.

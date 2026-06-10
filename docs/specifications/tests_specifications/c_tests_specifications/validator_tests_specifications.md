# Test Documentation — Validator Module (`validator.c`)

This document describes the unit testing suite developed to evaluate the business rules engine used on market entries (`validator.c`). The module works under an isolated pattern, evaluating copies passed by value and passing the validation verdict via a boolean parameter reference.

## Implemented Test Cases

### 1. NULL Output Pointer Resilience
* **Target Function:** `vld_validate_order`
* **Objective:** Ensure that passing an unallocated or null output pointer address does not crash the system.
* **Verification:** Supplies a valid order structure but passes `NULL` as the boolean output reference, asserting that the function responds with an infrastructure error token `ERR_ORD`.

### 2. Status Execution Path Parity
* **Target Function:** `vld_validate_order`
* **Objective:** Ensure that any regular execution track returns a neutral code state token (`ERR_NONE`).
* **Verification:** Asserts that checking an entry against a regular stack boolean reference address successfully yields `0`.

### 3. Preservation of Clean Structs
* **Target Function:** `vld_validate_order`
* **Objective:** Validate that perfectly structured orders leave the output boolean set as positive.
* **Verification:** Submits two correct orders (a Sell order 'A' and a Buy order 'B') with valid positive prices and quantities, asserting that the out reference evaluates to `true` in both instances.

### 4. Rejection of Price Zero Boundary
* **Target Function:** `vld_validate_order`
* **Objective:** Disallow orders trying to enter the engine with a null transaction value.
* **Verification:** Populates an order with `price = 0.0` and asserts that the validation output reference falls back to `false`.

### 5. Rejection of Negative Price
* **Target Function:** `vld_validate_order`
* **Objective:** Invalidate any market row trying to feed negative price metrics into the order book.
* **Verification:** Feeds an entry configured with `price = -100.0` and asserts that the output verdict yields `false`.

### 6. Rejection of Empty Quantity Volume
* **Target Function:** `vld_validate_order`
* **Objective:** Enforce the trading constraint that mandates a minimum volume size of at least 1 lot.
* **Verification:** Configures an order with `quantity = 0` and asserts that the resulting target boolean evaluates to `false`.

### 7. Rejection of Rogue Side Characters
* **Target Function:** `vld_validate_order`
* **Objective:** Invalidate entries that violate the domain boundaries for transaction sides.
* **Verification:** Evaluates a struct with `side = 'X'`, verifying that the evaluation returns `false` due to a domain rule violation.

### 8. Prior Parser Rejection Normalization
* **Target Function:** `vld_validate_order`
* **Objective:** Guarantee that orders marked as structurally invalid by the parser stage (`is_valid = false`) are blocked instantly without processing their underlying values.
* **Verification:** Configures an item with correct values but flags it with `.is_valid = false`, asserting that the returned reference is immediately evaluated as `false`.

### 9. Mixed Sequential Verification
* **Target Function:** `vld_validate_order`
* **Objective:** Verify loop stability when evaluating varying sequences of clean and broken structures sequentially.
* **Verification:** Iterates the routine across four consecutive items containing one clean and three broken records, asserting the exact boolean array layout match (`[True, False, False, False]`).

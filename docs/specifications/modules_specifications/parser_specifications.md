# Technical Specifications: Parser Library Module

## 1. prs_create_orders

* **a) Objective:**
    * Parses input market data CSV files, maps validation statuses, and builds dynamic order arrays on the heap.
* **b) Functional Requirements:**
    * Must perform a strict 2-pass operational sequence: Pass 1 calculates non-header data row limits; Pass 2 reads layouts sequentially using formatted standard parsing rules (`sscanf`).
    * Must skip the initial descriptive character row column header string layout automatically.
    * Must dynamically allocate `orders_array` to match the measured row footprint count precisely.
    * Must invoke `vld_validate_order` on every line. Elements passing are saved with `is_valid = true`. Records failing parsing or formatting parameters are retained as structural placeholders with fields overwritten to `order_id = -1` and `is_valid = false`.
* **c) Coupling:**
    * **Parameters:**
        * `csv_path` [Input]: Character path string targeting a file asset.
        * `handle` [Output]: Target memory double pointer updated with the generated parser context address.
        * `total_count` [Output]: Memory reference integer capturing the parsed read boundary counts.
    * **Returns:**
        * `ERR_NONE`: Parsing and setup sequences processed cleanly.
        * `ERR_ORD`: Underlying physical file is unreadable, missing, or lacks valid string structure lines.
        * `ERR_MEM`: System dynamic memory allocation parameters or array configuration setups failed.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `csv_path != NULL`
        * `handle != NULL`
        * `total_count != NULL`
    * **Post-conditions (Output Assertions):**
        * `*handle != NULL`
        * `(*handle)->count == *total_count`
        * `(*handle)->orders_array != NULL`
* **e) Assumptions and Restrictions:**
    * Expects a standard comma-separated file format containing consistent columns matching the schema fields layout of `obk_order_t`.

---

## 2. prs_get_order_by_index

* **a) Objective:**
    * Extracts an isolated copy-by-value of a specific parsed order row layout from the storage array.
* **b) Functional Requirements:**
    * Must verify target access coordinates against internal tracker bounds (`0 <= idx < count`).
    * Must perform direct value attribute extraction to prevent internal state pointer leaks.
* **c) Coupling:**
    * **Parameters:**
        * `handle` [Input]: Operational parser instance pointer handle.
        * `idx` [Input]: Array tracking position integer coordinate.
        * `out_order` [Output]: Destination order struct target layout pointer (`obk_order_pt`).
    * **Returns:**
        * `ERR_NONE`: Memory duplicate assignment finalized natively.
        * `ERR_ORD`: Boundary parameters were breached (`idx < 0` or `idx >= count`), or reference handle pointer parameters are invalid.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `handle != NULL`
        * `out_order != NULL`
        * `idx >= 0 && idx < handle->count`
    * **Post-conditions (Output Assertions):**
        * `out_order->order_id == handle->orders_array[idx].order_id`
        * `out_order->is_valid == handle->orders_array[idx].is_valid`
        * `out_order->quantity == handle->orders_array[idx].quantity`

---

## 3. prs_free_buffer

* **a) Objective:**
    * Cleans the active parsing environment instances, freeing internal row layout arrays and wrappers.
* **b) Functional Requirements:**
    * Must safely free the inner dynamic block point array (`orders_array`) if populated.
    * Must free the central instance wrapper container allocation block back to the global environment and clear pointers.
* **c) Coupling:**
    * **Parameters:**
        * `handle` [Input]: The operational parser context pointer instance to destroy.
    * **Returns:**
        * `ERR_NONE`: Free sequences concluded without internal errors.
        * `ERR_ORD`: Passed tracking reference parameter layout was already `NULL`.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `handle != NULL`
    * **Post-conditions (Output Assertions):**
        * Target parser memory zones are completely unmapped and safely returned to the heap.

# Technical Specifications: Matching Engine Module

## 1. mtc_create_engine

* **a) Objective:**
    * Allocates, initializes, and configures a new opaque financial matching engine instance.
* **b) Functional Requirements:**
    * Must allocate dynamic memory for the internal private state structure `struct mtc_instance_private_s`.
    * Must internally initialize an underlying order book instance using `obk_initialize_book`.
    * Must explicitly initialize internal transaction trackers (`trade_id`, `trades_count`, and `last_processed_idx`) to zero.
* **c) Coupling:**
    * **Parameters:**
        * `handle` [Output]: A double pointer (`mtc_handle_pt*`) that will be updated to hold the address of the newly allocated matching engine context.
    * **Returns:**
        * `ERR_NONE`: Engine allocation and underlying order book instantiation succeeded.
        * `ERR_MEM`: Provided destination handle reference is null, or standard heap `malloc` failed to reserve memory space.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `handle != NULL`
    * **Post-conditions (Output Assertions):**
        * `*handle != NULL`
        * `(*handle)->trades_count == 0`
        * `(*handle)->trade_id == 0`
        * `(*handle)->last_processed_idx == 0`
* **e) Assumptions and Restrictions:**
    * Relies directly on heap stability and successful execution of `obk_initialize_book`.

---

## 2. mtc_process_matching

* **a) Objective:**
    * Sequentially processes parsed active market orders, matches crossing prices, and updates the execution state.
* **b) Functional Requirements:**
    * Must resume iteration from `last_processed_idx` up to the total order bound, automatically skipping rows where `is_valid == false`.
    * Must validate incoming active orders against the top-priority resting order of the opposite side heap via `obk_get_order`.
    * Must strictly enforce the **Self-Trading Rule**: if an active order matches a resting order with the same client identifier (`buy_client_id == sell_client_id`), the active order must be discarded immediately, breaking its cross-execution loop to prevent wash trading.
    * Must apply the **Passive Order Price Strategy**: matching transaction entries must record the execution price of the resting order already standing inside the book.
    * Must update structural book elements on execution: full fills clear the resting order via `obk_remove_order`, partial fills modify the remaining volume using `obk_change_order`, and unexecuted residual active balances are pushed into the book using `obk_insert_order`.
    * Must write verified transactions into the internal storage array up to `MTC_MAX_TRADES` (8192) and increment `trades_count`.
* **c) Coupling:**
    * **Parameters:**
        * `handle` [Input/Output]: Opaque pointer handle targeting the operational matching engine instance.
        * `prs_handle` [Input]: Opaque handle pointing to the populated CSV parser memory buffer structure.
        * `total_orders` [Input]: Integer representing total parsed order entries to evaluate.
        * `out_total_trades` [Output]: Destination integer pointer receiving the finalized tracking counts.
    * **Returns:**
        * `ERR_NONE`: Entire loop sequence executed natively without system faults.
        * `ERR_ORD`: Encountered an invalid internal engine status, or reference input components are missing.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `handle != NULL`
        * `prs_handle != NULL`
        * `out_total_trades != NULL`
        * `handle->trades_count <= 8192`
    * **Post-conditions (Output Assertions):**
        * `*out_total_trades == handle->trades_count`
        * `handle->last_processed_idx == total_orders`
        * Underlying order book structural sorting invariants (Min-Heap / Max-Heap) remain intact.
* **e) Assumptions and Restrictions:**
    * Processing assumes that the data in `prs_handle` matches the layout specified in `parserlib.h` and has been validated prior to the matching phase.

---

## 3. mtc_get_trade_by_index

* **a) Objective:**
    * Fetches an isolated copy-by-value of a validated transaction record from the engine's internal array layout by its index.
* **b) Functional Requirements:**
    * Must evaluate boundary parameters to prevent memory buffer underflows or overflows.
    * Must accurately duplicate structural data attributes directly to the output target address without sharing private internal state pointers.
* **c) Coupling:**
    * **Parameters:**
        * `handle` [Input]: Opaque matching engine context handle instance.
        * `idx` [Input]: Targeted transaction array index coordinate integer.
        * `out_trade` [Output]: Destination transaction layout pointer (`mtc_transaction_t*`) receiving data.
    * **Returns:**
        * `ERR_NONE`: Extraction process completed successfully.
        * `ERR_ORD`: Given array index falls outside active tracking ranges (`idx < 0` or `idx >= trades_count`), or parameter references are null.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `handle != NULL`
        * `out_trade != NULL`
        * `idx >= 0 && idx < handle->trades_count`
    * **Post-conditions (Output Assertions):**
        * `out_trade->trade_id == handle->trades_array[idx].trade_id`
        * `out_trade->price == handle->trades_array[idx].price`
        * `out_trade->quantity == handle->trades_array[idx].quantity`

---

## 4. mtc_clear_engine

* **a) Objective:**
    * Safely releases matching engine structural wrapper components, dynamic trade vectors, and internal order books.
* **b) Functional Requirements:**
    * Must trigger `obk_clear_book` on the inner abstract book pointer container to clear active heaps safely.
    * Must completely free the main layout allocation wrapper block back to the heap and assign target references back to `NULL`.
* **c) Coupling:**
    * **Parameters:**
        * `handle` [Input/Output]: Double pointer instance referencing the operational matching engine context to erase.
    * **Returns:**
        * `ERR_NONE`: System memory cleaning finished completely without exceptions.
        * `ERR_MEM`: The double pointer context wrapper or underlying target `*handle` was already `NULL`.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `handle != NULL`
        * `*handle != NULL`
    * **Post-conditions (Output Assertions):**
        * `*handle == NULL`

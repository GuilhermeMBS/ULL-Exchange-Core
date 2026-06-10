# Software Contracts: Pre-conditions and Post-conditions

This document formally establishes the software contracts (Design by Contract) for the public functions of the Financial Exchange Matching Engine, specifying input constraints, output guarantees, and structural invariants based on the current implementation.

---

## 1. Module `book.h` / `book.c`

### Structural Invariants of the Order Book (`struct obk_book_private_s`)
* `size_asks` must always remain within the range $[0, 4096]$ (`BUFFER_SIZE`).
* `size_bids` must always remain within the range $[0, 4096]$ (`BUFFER_SIZE`).
* The `asks` array must strictly satisfy the **Min-Heap** property:
  * Primary criterion: Lowest price (`price`) at the root ($i = 0$).
  * Secondary criterion (tie-breaker): Earliest timestamp (`timestamp`).
* The `bids` array must strictly satisfy the **Max-Heap** property:
  * Primary criterion: Highest price (`price`) at the root ($i = 0$).
  * Secondary criterion (tie-breaker): Earliest timestamp (`timestamp`).

### `obk_initialize_book`
* **Pre-conditions:**
  * `book != NULL` (The double pointer to receive the allocated book instance must be valid).
* **Post-conditions:**
  * **Success (`ERR_NONE`):** `*book` points to a valid heap-allocated structure. `size_asks = 0` and `size_bids = 0`. Internal order arrays are fully cleared/zeroed out.
  * **Error (`ERR_MEM`):** Triggered if heap memory allocation fails. `*book` remains unmodified.

### `obk_clear_book`
* **Pre-conditions:**
  * `book != NULL` and `*book != NULL` (The instance handle to deallocate must be valid).
* **Post-conditions:**
  * **Success (`ERR_NONE`):** The inner structural allocations and the book instance itself are deallocated via `free()`. The target pointer is safely nullified (`*book = NULL`).
  * **Error (`ERR_MEM`):** Triggered if `book` or `*book` are already `NULL`.

### `obk_copy_order`
* **Pre-conditions:**
  * `cpy != NULL` (Destination structure pointer must be valid).
  * `buffer != NULL` (Source array pointer must be valid).
  * `idx` must be a valid non-negative index within the source array bounds.
* **Post-conditions:**
  * Returns `0`. Data fields (`client_id`, `order_id`, `price`, `quantity`, `side`, and `timestamp`) from `buffer[idx]` are copied directly into the `cpy` structural target.

### `obk_insert_order`
* **Pre-conditions:**
  * `book != NULL` (Active order book instance handle).
  * `cpy != NULL` (The structure data containing the order to insert must be valid).
  * `cpy->side` must be exactly `'A'` (Ask/Sell) or `'B'` (Bid/Buy).
* **Post-conditions:**
  * **Success (`ERR_NONE`):** The order is appended to the tail of the matching side heap. The internal `obk_heapify` function is called from the inserted element's index upward to maintain heap invariants. The counter (`size_asks` or `size_bids`) is incremented by exactly 1.
  * **Error (`ERR_ORD`):** Triggered if the `side` character descriptor is neither `'A'` nor `'B'`. Book structures remain unaltered.
  * **Error (`ERR_MEM`):** Triggered if the target side heap is full (`size >= BUFFER_SIZE`). No structure changes are made.

### `obk_remove_order`
* **Pre-conditions:**
  * `book != NULL` (Active order book instance handle).
  * `side` must be exactly `'A'` or `'B'`.
  * The target book side counter (`size_asks` or `size_bids`) must be $> 0$ (Underflow prevention).
* **Post-conditions:**
  * **Success (`ERR_NONE`):** The side order counter is decremented by 1. The top-level priority element ($i = 0$) is replaced by the last element in the array, and the heap balance is restabilized by calling `obk_heapify` downwards starting from root index 0.
  * **Error (`ERR_ORD`):** Triggered if `side` descriptor is invalid or if the targeted side heap is empty.

### `obk_change_order`
* **Pre-conditions:**
  * `book != NULL` (Active order book instance handle).
  * `side` must be exactly `'A'` or `'B'`.
  * The corresponding side heap must contain at least one valid active order ($i = 0$).
* **Post-conditions:**
  * **Success (`ERR_NONE`):** The volume property (`quantity`) of the highest priority order ($i = 0$) is directly overwritten with the value of `qty`. The internal `obk_heapify` routine executes from index 0 downwards to ensure priority sequence order remains valid.
  * **Error (`ERR_ORD`):** Triggered if the `side` character value is invalid.

### `obk_get_order`
* **Pre-conditions:**
  * `book != NULL`.
* **Post-conditions:**
  * Returns an isolated structural copy-by-value (`obk_order_t`). If `side` is valid and the book is not empty, it returns the top-level element ($i = 0$). If invalid, it returns a zeroed-out structural template. The underlying state parameters of the book are not modified (Peek operation).

### `obk_ask_count` and `obk_bid_count`
* **Pre-conditions:**
  * None (The implementation verifies if `book` is null internally).
* **Post-conditions:**
  * Returns the current tracking count integer (`size_asks` or `size_bids`). Returns `0` if the passed handle parameter is `NULL`.

---

## 2. Module `matching.h` / `matching.c`

### Structural Invariants of the Matching Engine (`struct mtc_instance_private_s`)
* `trades_count` must always stay within the static bounds $[0, 8192]$ (`MTC_MAX_TRADES`).
* Execution Price Strategy: When a trade match occurs, the final price (`mtc_transaction_t.price`) is strictly dictated by the **passive order price** (the resting order that was already standing inside the book).
* Self-Trading Rule: Transacting with oneself is strictly prohibited (`buy_client_id != sell_client_id`).

### `mtc_create_engine`
* **Pre-conditions:**
  * `handle != NULL` (Destination memory reference to hold the engine handle address).
* **Post-conditions:**
  * **Success (`ERR_NONE`):** `*handle` points to an allocated engine structure instance. An inner order book instance is instantiated via `obk_initialize_book`. Trackers `trade_id`, `trades_count`, and `last_processed_idx` are initialized to 0.
  * **Error (`ERR_MEM`):** Triggered if heap memory allocation for either the engine container wrapper or the internal book tracker fails.

### `mtc_process_matching`
* **Pre-conditions:**
  * `handle != NULL` (Initialized engine instance).
  * `prs_handle != NULL` (Populated parser memory reference).
  * `out_total_trades != NULL` (Valid integer reference pointer to receive the trade execution counts).
* **Post-conditions:**
  * **Success (`ERR_NONE`):** The matching processing cycle loops from `last_processed_idx` up to `total_orders`. Orders marked with `is_valid == false` are skipped.
  * Incoming active orders match against the best available resting order on the opposite book side:
    * If prices cross and `client_id` markers are different, a transaction is written into `trades_array` and `trades_count` increments.
    * Full execution of a resting order clears it out via `obk_remove_order`.
    * Partial execution edits the remaining balance via `obk_change_order`.
    * If an incoming order has remaining volume after a match, it recursively checks against subsequent book orders. If no further cross is possible, the remaining residue is stored in the book via `obk_insert_order`.
    * **Self-Trading Rejection:** If a cross matches matching client IDs (`client_id` match), the newly incoming active order is **discarded** immediately (breaking the execution loop for that specific order and returning an internal order handling indicator).
  * Upon completion, `*out_total_trades` is updated with the value of `trades_count`.

### `mtc_get_trade_by_index`
* **Pre-conditions:**
  * `handle != NULL` and `out_trade != NULL`.
  * `idx` must strictly fulfill the array boundary condition $0 \le \text{idx} < \text{trades->count}$.
* **Post-conditions:**
  * **Success (`ERR_NONE`):** The transaction record at index `idx` inside `trades_array` is copied directly into the `out_trade` target address.
  * **Error (`ERR_ORD`):** Triggered if `idx` falls out of active bounds or if parameter references are null.

### `mtc_clear_engine`
* **Pre-conditions:**
  * `handle != NULL` and `*handle != NULL`.
* **Post-conditions:**
  * **Success (`ERR_NONE`):** Triggers `obk_clear_book` on the internal book container, frees the main engine allocation wrapper block, and resets the handle target reference back to `NULL`.
  * **Error (`ERR_ORD`):** Triggered if the handle pointer context or reference contents are invalid.

---

## 3. Module `parserlib.h` / `parserlib.c`

### Structural Invariants of the CSV Parser (`struct prs_instance_private_s`)
* `count` represents the exact quantity of successfully read entries from the file, excluding the top CSV string header.
* `orders_array` points to a dynamic heap block sized accurately matching the total row count parsed during the initial structural pre-scan loop.

### `prs_create_orders`
* **Pre-conditions:**
  * `csv_path != NULL` (Target path string mapping to an existing file location).
  * `handle != NULL` and `total_count != NULL`.
  * The file must start with a descriptive header string (which is skipped automatically).
* **Post-conditions:**
  * **Success (`ERR_NONE`):** Performs a 2-pass scan. Pass 1 counts rows. Pass 2 populates `orders_array`. Lines are read using a fixed format layout parsed via `sscanf`. Valid rows are checked via `vld_validate_order` and saved with `is_valid = true`. Lines failing format parsing or validation are kept with structural placeholders where `order_id = -1` and `is_valid = false`. `*total_count` and `*handle` are set to the matching read bounds.
  * **Error (`ERR_ORD`):** Triggered if the target file cannot be accessed or is structurally empty.
  * **Error (`ERR_MEM`):** Triggered if heap configuration setups or array memory reservations fail.

### `prs_get_order_by_index`
* **Pre-conditions:**
  * `handle != NULL` and `out_order != NULL`.
  * `idx` must adhere to the boundary constraint $0 \le \text{idx} < \text{instance->count}$.
* **Post-conditions:**
  * **Success (`ERR_NONE`):** Performs a copy-by-value extraction of the order structure located at `orders_array[idx]` into the `out_order` target address.
  * **Error (`ERR_ORD`):** Triggered if data boundaries are breached or reference inputs are invalid.

### `prs_free_buffer`
* **Pre-conditions:**
  * `handle != NULL`.
* **Post-conditions:**
  * **Success (`ERR_NONE`):** Frees the dynamic allocations pointing to `orders_array` (if populated), clears the tracking instance structure wrapper from the heap, and yields a success indicator.

---

## 4. Module `ledger.h` / `ledger.c`

### `ldg_save_ledger`
* **Pre-conditions:**
  * `bin_path != NULL` (Target string for output database creation).
  * `mtc_handle != NULL` (Populated engine state wrapper tracking transactions).
  * `total_trades` must match the processed execution count tracked at the end of engine matching routines.
* **Post-conditions:**
  * **Success (`ERR_NONE`):** Opens a destination binary handle in write-block mode (`"wb"`). Loops sequentially from index $0$ to `total_trades - 1`, fetching data entries via `mtc_get_trade_by_index` and dumping raw binary footprints to disk using single-block `fwrite` operations. Applies `fflush()` to commit physical storage before closing handles.
  * **Error (`ERR_ORD`):** Triggered if path parameters are missing or file handle allocations fail on disk.
  * **Error (`ERR_MEM`):** Triggered if file-write items returned from `fwrite` deviate from the expected structure unit footprint count, closing files immediately.

---

## 5. Module `validator.h` / `validator.c`

### `vld_validate_order`
* **Pre-conditions:**
  * `out_is_valid != NULL` (Boolean reference target pointer cannot be null).
* **Post-conditions:**
  * **Success (`ERR_NONE`):** Evaluates properties on the copy-by-value `order` input:
    * If `order.is_valid` is initially false, sets `*out_is_valid = false` and exits immediately.
    * Otherwise, enforces engine trade compliance specifications:
      1. `order.price` must be strictly $> 0.0$.
      2. `order.quantity` must be $\ge 1$.
      3. `order.side` must match exactly character values `'A'` or `'B'`.
    * Sets `*out_is_valid = true` if all rules pass successfully; otherwise, returns `false`.
  * **Error (`ERR_ORD`):** Triggered if the destination reference argument `out_is_valid` is a null pointer.

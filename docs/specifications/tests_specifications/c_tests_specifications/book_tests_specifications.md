# Test Documentation — Book Module (`book.c`)

This document describes the unit testing suite developed to validate the structural integrity, data isolation, and Price/Time priority sorting logic of the Opaque Order Book structure (`obk_book_pt`). The module manages separate internal Heap structures for both buy orders (Bids) and sell orders (Asks).

## Implemented Test Cases

### 1. Initialization and Instance Isolation
* **Target Function:** `obk_initialize_book` and `obk_insert_order`
* **Objective:** Verify that memory allocation for new opaque book instances succeeds (`ERR_NONE`) and that rigorous data segregation is maintained between independent handles.
* **Verification:** Inserts an order into `Book A` and asserts through introspection that queries against `Book B` remain completely unaffected and empty.

### 2. Basic Buy Order Sorting Priority (Bids)
* **Target Function:** `obk_insert_order` and `obk_get_order`
* **Objective:** Validate the Price/Time priority queuing criteria for buy orders (Bids).
* **Verification:** Inserts three distinct orders with varying prices and timestamps. Asserts that the top of the internal heap structure always references the **highest price**. In case of a price tie, it asserts that the tie-breaker rule prioritizes the **lowest timestamp** (the oldest order).

### 3. Basic Sell Order Sorting Priority (Asks)
* **Target Function:** `obk_insert_order` and `obk_get_order`
* **Objective:** Validate the Price/Time priority queuing criteria for sell orders (Asks).
* **Verification:** Inserts three distinct orders with varying prices. Asserts that the top of the internal heap structure always references the **lowest price** (the most competitive selling offer). In case of a price tie, it verifies that the oldest entry (**lowest timestamp**) is prioritized at the top.

### 4. Sequential Modification and Pop Mechanics
* **Target Function:** `obk_change_order` and `obk_remove_order`
* **Objective:** Validate the runtime routines responsible for altering order volumes and discarding items from the top of the heap.
* **Verification:** Modifies the quantity of the highest priority order using `obk_change_order` and asserts the new volume. Then, consecutively pops entries via `obk_remove_order` and verifies that the next available candidate correctly promotes to the top of the heap tree structure.

### 5. Boundary Limits and Allocation Saturation
* **Target Function:** `obk_insert_order` and `obk_clear_book`
* **Objective:** Stress test the hard allocation threshold boundaries of the internal static memory buffer.
* **Verification:** Loops exactly `4096` consecutive insertion cycles to fully saturate the layout capacity limits (`BUFFER_SIZE`). Attempts to insert a `4097th` order and asserts that the implementation gracefully rejects the operation by returning `ERR_MEM` safely without leaking memory or triggering segmentation faults. Cleans resources using `obk_clear_book`.

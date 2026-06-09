# Technical Specifications: Order Book Module

## 1. obk_initialize_book

* **a) Objective:**
    * Allocates, initializes, and blanks out a new opaque order book instance.
* **b) Functional Requirements:**
    * Must allocate memory for the internal private structure `struct obk_book_private_s`.
    * Must initialize internal heap sizes (`size_asks` and `size_bids`) explicitly to zero.
    * Must safely clear all array buffers (`asks` and `bids`), zeroing structural values and setting `is_valid` states to `false`.
* **c) Coupling:**
    * **Parameters:**
        * `book` [Output]: A double pointer (`obk_book_pt*`) that will be updated to point to the address of the newly allocated memory area.
    * **Returns:**
        * `ERR_NONE`: Memory allocation and initial value structures succeeded.
        * `ERR_MEM`: Provided reference pointer is null, or `malloc` engine failed to allocate memory space.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `book != NULL`
    * **Post-conditions (Output Assertions):**
        * `*book != NULL`
        * `(*book)->size_asks == 0`
        * `(*book)->size_bids == 0`
* **e) Assumptions and Restrictions:**
    * Relies directly on heap stability and standard C library dynamic allocator availability (`malloc`).

---

## 2. obk_clear_book

* **a) Objective:**
    * Safely releases allocated heap memory back to the environment and cleans instance pointers.
* **b) Functional Requirements:**
    * Must completely deallocate the underlying heap block targeted by the opaque internal structures.
    * Must clear and assign the pointer location value back to `NULL` to eliminate dangling reference exploits or errors.
* **c) Coupling:**
    * **Parameters:**
        * `book` [Input/Output]: A double pointer instance (`obk_book_pt*`) referencing the operational data structures to erase.
    * **Returns:**
        * `ERR_NONE`: Free operations completed without exceptions.
        * `ERR_MEM`: The wrapper double pointer or the instance context targeted at `*book` was already `NULL`.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `book != NULL`
        * `*book != NULL`
    * **Post-conditions (Output Assertions):**
        * `*book == NULL`

---

## 3. obk_copy_order

* **a) Objective:**
    * Performs an explicit field-by-field memory transfer of a specific order entry from a source array buffer to a destination target structure.
* **b) Functional Requirements:**
    * Must accurately copy data members (`client_id`, `order_id`, `price`, `quantity`, `side`, `timestamp`) without pointer sharing.
* **c) Coupling:**
    * **Parameters:**
        * `cpy` [Output]: Target order struct pointer (`obk_order_pt`) receiving copied attributes.
        * `buffer` [Input]: Source sequential array container pointer (`obk_order_pt`) housing operational records.
        * `idx` [Input]: Array indexing value integer (`int32_t`) representing the operational read coordinate.
    * **Returns:**
        * `0` (Success): Structural copying process concluded natively.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `cpy != NULL`
        * `buffer != NULL`
        * `idx >= 0`
    * **Post-conditions (Output Assertions):**
        * `cpy->order_id == buffer[idx].order_id`
        * `cpy->price == buffer[idx].price`
        * `cpy->quantity == buffer[idx].quantity`

---

## 4. obk_insert_order

* **a) Objective:**
    * Appends an order to the matching matrix, routing to the correct execution book side, and triggers a structural re-sort.
* **b) Functional Requirements:**
    * Must identify execution side maps by assessing explicit side characters (`'A'` or `'B'`).
    * Must validate allocation saturation barriers to guarantee total entries do not cross structural thresholds (`BUFFER_SIZE`).
    * Must insert data at the bottom of the structural list and trigger the generic internal tracking algorithm (`obk_heapify`) to reposition elements.
* **c) Coupling:**
    * **Parameters:**
        * `book` [Input/Output]: Target abstract handle (`obk_book_pt`) representing the central market matching simulation data framework.
        * `cpy` [Input]: Pointer reference layout containing incoming raw order configurations to register.
    * **Returns:**
        * `ERR_NONE`: System correctly queued and stabilized the target data matrix location.
        * `ERR_ORD`: Execution side variable did not match accepted values (`'A'` or `'B'`).
        * `ERR_MEM`: Book tracking arrays have reached their internal hard ceiling capacity limit (`4096`).
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `book != NULL`
        * `cpy != NULL`
        * `cpy->side == 'A' || cpy->side == 'B'`
        * `(cpy->side == 'A' && book->size_asks < BUFFER_SIZE) || (cpy->side == 'B' && book->size_bids < BUFFER_SIZE)`
    * **Post-conditions (Output Assertions):**
        * For Bid: `book->size_bids == initial(book->size_bids) + 1`
        * For Ask: `book->size_asks == initial(book->size_asks) + 1`
        * Order book structure meets standard structural sorting invariants.

---

## 5. obk_remove_order

* **a) Objective:**
    * Removes the highest-priority order at index `0` and stabilizes the underlying structural representation.
* **b) Functional Requirements:**
    * Must parse specific side selectors and check that matching books contain valid records (`size > 0`).
    * Must swap the tail record into the root position (`index 0`), decrement active tracking tallies, and invoke standard down-heap actions via `obk_heapify`.
* **c) Coupling:**
    * **Parameters:**
        * `book` [Input/Output]: Opaque handle instance pointer (`obk_book_pt`) hosting trading parameters.
        * `side` [Input]: Structural identifier selector character defining the book tracking structure (`'A'` or `'B'`).
    * **Returns:**
        * `ERR_NONE`: Root item cleared and structure successfully normalized.
        * `ERR_ORD`: Received invalid side parameters or the requested data container is completely empty.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `book != NULL`
        * `side == 'A' || side == 'B'`
        * `(side == 'A' && book->size_asks > 0) || (side == 'B' && book->size_bids > 0)`
    * **Post-conditions (Output Assertions):**
        * For Bid: `book->size_bids == initial(book->size_bids) - 1`
        * For Ask: `book->size_asks == initial(book->size_asks) - 1`
        * Root node holds the next prioritized item.

---

## 6. obk_change_order

* **a) Objective:**
    * Amends the volume (quantity) properties of the top-priority order inside a designated side heap.
* **b) Functional Requirements:**
    * Must replace the volume matrix component at `index 0` with the newly provided volume argument.
    * Must trigger an active structural re-sort using `obk_heapify` from root index position `0` to reposition the modified entry if needed.
* **c) Coupling:**
    * **Parameters:**
        * `book` [Input/Output]: Abstract instance pipeline pointer (`obk_book_pt`).
        * `qty` [Input]: New unsigned volume context integer (`uint32_t`) to patch inside the matching index.
        * `side` [Input]: Character parameter selector defining structural targets (`'A'` or `'B'`).
    * **Returns:**
        * `ERR_NONE`: Modification and structural updates finalized smoothly.
        * `ERR_ORD`: Passed invalid target character tags.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `book != NULL`
        * `side == 'A' || side == 'B'`
        * `(side == 'A' && book->size_asks > 0) || (side == 'B' && book->size_bids > 0)`
    * **Post-conditions (Output Assertions):**
        * For Bid: `book->bids[0].quantity == qty`
        * For Ask: `book->asks[0].quantity == qty`

---

## 7. obk_get_order

* **a) Objective:**
    * Safely returns an isolated, decoupled copy of the highest-priority order at the root of the designated side heap.
* **b) Functional Requirements:**
    * Must identify structural paths via side flags, and copy structural layout fields from root `index 0`.
    * Must pass a value-copied layout back to the consumer, avoiding pointer leaks to prevent external manipulation of internal states.
* **c) Coupling:**
    * **Parameters:**
        * `book` [Input]: Abstract target handle pointer instance (`obk_book_pt`).
        * `side` [Input]: Character execution side mapping value (`'A'` or `'B'`).
    * **Returns:**
        * `obk_order_t`: An isolated structural layout stack instance representing the prioritized element.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `book != NULL`
        * `side == 'A' || side == 'B'`
    * **Post-conditions (Output Assertions):**
        * Returned order copy retains identical fields matching the root state at `index 0` within the target data structure.
* **f) User Interface:**
    * If an invalid character or structure validation failure is intercepted by inner error pipelines, the system logs `"Order Error!"` to `stdout` and aborts runtime tracking using `exit(-1)` via `err_check_error`.

# Technical Specifications: Matching Engine Module

## Overview

The matching engine operates over a **module-level singleton** order book (`s_book`) of type `obk_order_book_t` and a monotonically incrementing trade counter (`s_trade_id`). All three public functions share this state implicitly — no book handle is passed by the caller.

### Return Codes

| Code | Meaning |
|------|---------|
| `ERR_NONE (0)` | No match occurred — order was inserted into the book. |
| `1` | Full match — incoming order was completely executed. |
| `2` | Partial match — incoming order was partially executed; remainder may still be in the book. |
| `ERR_ORD (-1)` | Invalid order — null pointer or unrecognized side character. |

### Transaction Record

Each match event produces an `mtc_transaction_t` value on the stack with the following fields:

| Field | Type | Description |
|-------|------|-------------|
| `timestamp` | `uint32_t` | Unix epoch second at which the match occurred. |
| `trade_id` | `int32_t` | Unique, auto-incremented identifier for this trade. |
| `buy_order_id` | `int32_t` | `order_id` of the buy-side order involved. |
| `sell_order_id` | `int32_t` | `order_id` of the sell-side order involved. |
| `buy_client_id` | `int32_t` | `client_id` of the buyer. |
| `sell_client_id` | `int32_t` | `client_id` of the seller. |
| `price` | `double` | Execution price (always the resting order's price). |
| `quantity` | `int32_t` | Quantity exchanged in this single match event. |

---

## 1. mtc_make_trade

* **a) Objective:**
    * Entry point for the matching engine. Routes an incoming order to the correct processing pipeline based on its execution side.
* **b) Functional Requirements:**
    * Must inspect `incoming->side` and delegate to `mtc_make_bid` when `side == 'B'` or to `mtc_make_sell` when `side == 'A'`.
    * Must reject null pointers and unrecognized side values without modifying internal state.
* **c) Coupling:**
    * **Parameters:**
        * `incoming` [Input]: Pointer to the order (`obk_order_t*`) to be processed by the engine.
    * **Returns:**
        * The return value of the delegated function (`mtc_make_bid` or `mtc_make_sell`).
        * `ERR_ORD`: `incoming` is `NULL` or `incoming->side` is neither `'A'` nor `'B'`.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `incoming != NULL`
        * `incoming->side == 'A' || incoming->side == 'B'`
    * **Post-conditions (Output Assertions):**
        * Internal state is unchanged if `ERR_ORD` is returned.
        * Otherwise, post-conditions are those of the delegated function.

---

## 2. mtc_make_bid

* **a) Objective:**
    * Processes an incoming **buy** order (`side == 'B'`) against the ask side of the singleton book, executing full or partial matches and recursing until the order is fully consumed or no further match is possible.
* **b) Functional Requirements:**
    * Must check whether the ask side of `s_book` contains any resting orders before attempting a match.
    * Must compare `order->price` against the best ask price; if the bid price is lower than the ask, no match is possible and the order must be inserted into the book.
    * Must calculate the executable quantity as `min(order->quantity, best_ask.quantity)`.
    * Must generate an `mtc_transaction_t` record for every match event, capturing both counterparty identifiers, the execution price, and the traded quantity.
    * Must handle three distinct match outcomes:
        * **Full match (quantities equal):** remove the resting ask from the book.
        * **Bid surplus (bid quantity greater):** remove the resting ask, decrement `order->quantity` by the traded amount, and recurse to attempt further matches.
        * **Bid deficit (bid quantity smaller):** reduce the resting ask's quantity via `changeOrder`; the ask remains in the book.
* **c) Coupling:**
    * **Parameters:**
        * `order` [Input/Output]: Pointer to the incoming buy order (`obk_order_t*`). The `quantity` field may be decremented in-place during recursive partial executions.
    * **Returns:**
        * `ERR_NONE (0)`: No match found — order was inserted into the book.
        * `1`: Order was fully matched and completely consumed.
        * `2`: Order was partially matched; the resting ask absorbed the remainder.
        * `ERR_ORD (-1)`: `order` is `NULL`.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `order != NULL`
        * `order->side == 'B'`
    * **Post-conditions (Output Assertions):**
        * If `ERR_NONE`: `s_book.size_asks == initial(s_book.size_asks) + 1`.
        * If `1` (full match, equal quantities): `s_book.size_asks == initial(s_book.size_asks) - 1`.
        * If `1` (full match via recursion): `s_book.size_asks` decreased by the number of asks consumed across all recursive calls.
        * If `2`: `s_book.size_asks` is unchanged; `s_book.asks[0].quantity` was reduced by the traded quantity.
        * `s_trade_id` is incremented by one for each match event that occurred.
* **e) Assumptions and Restrictions:**
    * Execution price is always taken from the **resting ask** (`best_ask.price`), not from the incoming bid.
    * Recursion depth is bounded by the number of resting asks in `s_book` (at most `BUFFER_SIZE`).

---

## 3. mtc_make_sell

* **a) Objective:**
    * Processes an incoming **sell** order (`side == 'A'`) against the bid side of the singleton book, executing full or partial matches and recursing until the order is fully consumed or no further match is possible.
* **b) Functional Requirements:**
    * Must check whether the bid side of `s_book` contains any resting orders before attempting a match.
    * Must compare `order->price` against the best bid price; if the ask price is higher than the bid, no match is possible and the order must be inserted into the book.
    * Must calculate the executable quantity as `min(order->quantity, best_bid.quantity)`.
    * Must generate an `mtc_transaction_t` record for every match event, capturing both counterparty identifiers, the execution price, and the traded quantity.
    * Must handle three distinct match outcomes:
        * **Full match (quantities equal):** remove the resting bid from the book.
        * **Ask surplus (ask quantity greater):** remove the resting bid, decrement `order->quantity` by the traded amount, and recurse to attempt further matches.
        * **Ask deficit (ask quantity smaller):** reduce the resting bid's quantity via `changeOrder`; the bid remains in the book.
* **c) Coupling:**
    * **Parameters:**
        * `order` [Input/Output]: Pointer to the incoming sell order (`obk_order_t*`). The `quantity` field may be decremented in-place during recursive partial executions.
    * **Returns:**
        * `ERR_NONE (0)`: No match found — order was inserted into the book.
        * `1`: Order was fully matched and completely consumed.
        * `2`: Order was partially matched; the resting bid absorbed the remainder.
        * `ERR_ORD (-1)`: `order` is `NULL`.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `order != NULL`
        * `order->side == 'A'`
    * **Post-conditions (Output Assertions):**
        * If `ERR_NONE`: `s_book.size_bids == initial(s_book.size_bids) + 1`.
        * If `1` (full match, equal quantities): `s_book.size_bids == initial(s_book.size_bids) - 1`.
        * If `1` (full match via recursion): `s_book.size_bids` decreased by the number of bids consumed across all recursive calls.
        * If `2`: `s_book.size_bids` is unchanged; `s_book.bids[0].quantity` was reduced by the traded quantity.
        * `s_trade_id` is incremented by one for each match event that occurred.
* **e) Assumptions and Restrictions:**
    * Execution price is always taken from the **resting bid** (`best_bid.price`), not from the incoming ask.
    * Recursion depth is bounded by the number of resting bids in `s_book` (at most `BUFFER_SIZE`).

# Technical Specifications: Ledger Module

## 1. ldg_save_ledger

* **a) Objective:**
    * Formats and commits verified transaction log arrays down to non-volatile disk targets using binary block writing.
* **b) Functional Requirements:**
    * Must open the target file directory location exclusively using write-block binary mode (`"wb"`).
    * Must loop sequentially from `0` up to `total_trades - 1`, gathering structural transaction frames using `mtc_get_trade_by_index`.
    * Must write raw structural binary bytes straight to disk surfaces using single-block `fwrite` operations.
    * Must call `fflush()` to force execution commits onto physical hardware devices before closing the active file channel stream to prevent layout data losses.
* **c) Coupling:**
    * **Parameters:**
        * `bin_path` [Input]: Destination database file directory string path.
        * `mtc_handle` [Input]: Opaque pointer tracking the populated matching engine status context.
        * `total_trades` [Input]: Total count of active transaction items captured during execution.
    * **Returns:**
        * `ERR_NONE`: Physical file system operations written and committed smoothly.
        * `ERR_ORD`: Target string paths are missing, or disk file stream setup operations failed.
        * `ERR_MEM`: File-write blocks returned by `fwrite` did not match the precise byte footprint of the target data structure unit, closing files immediately.
* **d) Coupling Conditions:**
    * **Pre-conditions (Input Assertions):**
        * `bin_path != NULL`
        * `mtc_handle != NULL`
        * `total_trades >= 0`
    * **Post-conditions (Output Assertions):**
        * Target ledger binary file exists on disk with a byte size matching exactly: `total_trades * sizeof(mtc_transaction_t)`.
* **e) Assumptions and Restrictions:**
    * Demands valid OS file write permissions on the targeted destination directory path block.

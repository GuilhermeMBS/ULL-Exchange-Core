# Makefile Execution Guide: Financial Exchange Engine

This document outlines instructions for working with the custom project build system (`Makefile`) to compile, run, and validate the system components.

---

## 🚀 Quick Reference Cheat-Sheet

For everyday development cycles, these three main commands cover the full execution context:

```bash
# 1. Run the main processing pipeline directly
make

# 2. Trigger the complete Continuous Integration (CI) test lifecycle
make test

# 3. Clean up compilation binaries and temporary data dumps
make clean

```

## 🛠️ Detailed Target Specification

### 1. Production Pipeline Integration
`make all` (or simply `make`)
- Description: The default top-level build target. Automatically commands execution to follow the `run` target routine.

- Operation: Launches the system orchestration scripts by activating `python3 main.py`.

`make run`
- Description: Explicit target pipeline execution entry rule.

- Operation: Standardizes console logs with execution banners and activates `python3 main.py`.

### 2. Native C Compilation & Testing Blocks
`make compile`
- Description: Compiles all module-level native C unit testing execution runners without running them.

- Operation: Invokes `gcc` using rigorous compilation flags (`-Wall -Wextra`) to produce 5 standalone test executables within the root working workspace directory:

    - `book_tester`

    - `ledger_tester`

    - `matching_tester`

    - `parser_tester`

    - `validator_tester`

`make test_c`
- Description: Automatically compiles and launches the native C verification suites.

- Operation: Ensures testing binaries match source file versions by calling `make compile`. Then, loops execution across each test runner binary sequentially to check memory alignment parameters, array boundaries, and sorting mechanics.

#### Isolating Individual Module Tests
If debugging a specific internal subsystem implementation block, you can bypass full testing loops by building target modules independently:

```bash
make book_tester      # Compiles solely the Order Book Heap logic verifier
make ledger_tester    # Compiles solely the Binary Persistence Ledger logic verifier
make matching_tester  # Compiles solely the Engine Matching logic verifier
make parser_tester    # Compiles solely the CSV Market Data Parser logic verifier
make validator_tester # Compiles solely the Data Validation compliance logic verifier
```

### 3. Python Integration & Workflows
`make test_python`
- Description: Performs structural behavioral checking on integration wrappers.

- Operation: Commands Python interpreters to verify operations on testing wrapper scripts:

    - `tests/python_tests/test_wrapper.py`

    - `tests/python_tests/test_market.py`

<br>

`make test_persistence`
- Description: Evaluates data persistence accuracy checks.

- Operation: Runs automated validation script `tests/python_tests/test_bin.py` to confirm binary transaction blocks created on disk match specifications.

### 4. Master Orchestrated Test Rule
`make test`
- Description: The core regression suite engine block. Ideal for pre-commit verification gates.

- Operation: Calls all verification layers sequentially in this order:

    1. `test_c` (Low-level native code unit testing)

    2. `test_python` (System-wide integration testing)

    3. `test_persistence` (Data persistence layout tracking)

### 5. Cleaning Workspace Objects
`make clean`
- Description: Clears temporary files and compilation dependencies out of directories.

- Operation: Wipes these objects permanently:

    - All 5 compiled tester binaries (`book_tester`, `ledger_tester`, etc.)

    - Executable items mapping to Windows runtime frameworks (`*.exe`)

    - Generated engine dynamic engine objects (`engine.so`)

    - Testing data artifacts (`data/test_ledger_temp.bin`)

## 📁 Required Project Directory Map
For the compiler flags (`-Icore/include`) and path targets to resolve correctly without path configuration errors, match your file tree to this layout:

```text
├── core/
│   └── src/
│       ├── retcodes.c
│       ├── book.c
│       ├── parserlib.c
│       ├── matching.c
│       ├── validator.c
│       └── ledger.c
├── core/include/        <-- Target for -I include headers search flags
├── tests/
│   ├── c_tests/
│   │   ├── test_book.c
│   │   ├── test_ledger.c
│   │   └── ...
│   └── python_tests/
│       ├── test_wrapper.py
│       ├── test_market.py
│       └── test_bin.py
├── main.py
└── Makefile
```

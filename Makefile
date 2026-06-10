# Compiler Configuration
CC = gcc
CFLAGS = -Wall -Wextra -Icore/include

# Project Directory Structures
SRC_DIR = core/src
TEST_DIR = tests/c_tests
PYTHON_TEST_DIR = tests/python_tests

# List of all test runner binaries
TESTERS = book_tester ledger_tester matching_tester parser_tester validator_tester

# Default target executed when running 'make' without arguments (Runs the production application)
all: run


# Individual Compilation Rules for Each Module
book_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(TEST_DIR)/test_book.c
	$(CC) $^ -o $@ $(CFLAGS)

ledger_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(SRC_DIR)/parserlib.c $(SRC_DIR)/matching.c $(SRC_DIR)/validator.c $(SRC_DIR)/ledger.c $(TEST_DIR)/test_ledger.c
	$(CC) $^ -o $@ $(CFLAGS)

matching_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(SRC_DIR)/parserlib.c $(SRC_DIR)/validator.c $(SRC_DIR)/matching.c $(TEST_DIR)/test_matching.c
	$(CC) $^ -o $@ $(CFLAGS)

parser_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(SRC_DIR)/parserlib.c $(SRC_DIR)/validator.c $(TEST_DIR)/test_parser.c
	$(CC) $^ -o $@ $(CFLAGS)

validator_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(SRC_DIR)/parserlib.c $(SRC_DIR)/validator.c $(TEST_DIR)/test_validator.c
	$(CC) $^ -o $@ $(CFLAGS)


# Utility target to compile all test runners without executing them
compile: $(TESTERS)


# Target to orchestrate and execute the full production application pipeline
run:
	@echo "=================================================="
	@echo "          LAUNCHING FULL EXCHANGE APPLICATION     "
	@echo "=================================================="
	@python3 main.py
	@echo "=================================================="


# Target to build and execute only the native C unit tests
test_c: compile
	@echo "=================================================="
	@echo "          RUNNING NATIVE C MODULE TESTS           "
	@echo "=================================================="
	@./book_tester
	@./ledger_tester
	@./matching_tester
	@./parser_tester
	@./validator_tester
	@echo "=================================================="
	@echo "        NATIVE C MODULE TESTS PASSED              "
	@echo "=================================================="


# Target to execute Python-level integration and generation tests
test_python:
	@echo "=================================================="
	@echo "          RUNNING PYTHON INTEGRATION TESTS        "
	@echo "=================================================="
	@python3 $(PYTHON_TEST_DIR)/test_wrapper.py
	@python3 $(PYTHON_TEST_DIR)/test_market.py
	@echo "=================================================="
	@echo "        PYTHON INTEGRATION TESTS PASSED           "
	@echo "=================================================="


# Target to specifically verify binary data ledger persistence tracks
test_persistence:
	@echo "=================================================="
	@echo "          RUNNING DATA PERSISTENCE TESTS          "
	@echo "=================================================="
	@python3 $(PYTHON_TEST_DIR)/test_bin.py
	@echo "=================================================="
	@echo "        DATA PERSISTENCE CHECKS PASSED            "
	@echo "=================================================="


# Main orchestrated target running all validation layers sequentially
test: test_c test_python test_persistence


# Artifact cleanup target
clean:
	rm -f $(TESTERS) *.exe engine.so data/test_ledger_temp.bin

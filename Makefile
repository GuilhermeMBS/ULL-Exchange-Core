# Compiler Configuration
CC = gcc
CFLAGS = -Wall -Wextra -Icore/include

# Project Directory Structures
SRC_DIR = core/src
TEST_DIR = tests/c_tests

# List of all test runner binaries
TESTERS = book_tester ledger_tester matching_tester parser_tester validator_tester

# Default target executed when running 'make' without arguments
all: test

# Individual Compilation Rules for Each Module
book_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(TEST_DIR)/test_book.c
	$(CC) $^ -o $@ $(CFLAGS)

ledger_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(SRC_DIR)/parser.c $(SRC_DIR)/matching.c $(SRC_DIR)/validator.c $(SRC_DIR)/ledger.c $(TEST_DIR)/test_ledger.c
	$(CC) $^ -o $@ $(CFLAGS)

matching_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(SRC_DIR)/parser.c $(SRC_DIR)/validator.c $(SRC_DIR)/matching.c $(TEST_DIR)/test_matching.c
	$(CC) $^ -o $@ $(CFLAGS)

parser_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(SRC_DIR)/parser.c $(SRC_DIR)/validator.c $(TEST_DIR)/test_parser.c
	$(CC) $^ -o $@ $(CFLAGS)

validator_tester: $(SRC_DIR)/retcodes.c $(SRC_DIR)/book.c $(SRC_DIR)/parser.c $(SRC_DIR)/validator.c $(TEST_DIR)/test_validator.c
	$(CC) $^ -o $@ $(CFLAGS)

# Utility target to compile all test runners without executing them
compile: $(TESTERS)

# Main target to build and run the entire validation suite in sequence
test: compile
	@echo "=================================================="
	@echo "          RUNNING ALL MODULAR TEST SUITES         "
	@echo "=================================================="
	@./book_tester
	@./ledger_tester
	@./matching_tester
	@./parser_tester
	@./validator_tester
	@echo "=================================================="
	@echo "          ALL MODULES COMPLETED SUCCESSFULLY      "
	@echo "=================================================="

# Artifact cleanup target
clean:
	rm -f $(TESTERS) *.exe

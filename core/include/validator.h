#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "retcodes.h"
#include "book.h"


/**
 * @brief Validates an individual order passed by value from the parser.
 * @param order Copy of the order data structure to be evaluated.
 * @param out_is_valid Pointer to a boolean that receives true if the order is valid, or false if it violates rules.
 * @return ERR_NONE on success, or ERR_ORD if the output pointer argument is invalid.
 * @note Rules: price or quantity <= 0, or side different from 'A' or 'B'.
 */
ret_code_t vld_validate_order(obk_order_t order, bool *out_is_valid);

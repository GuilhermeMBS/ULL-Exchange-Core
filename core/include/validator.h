#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "errorlib.h"
#include "book.h"

/*
Return: none (sets order_id to -1 on invalid orders).
Rules: price or quantity <= 0, or side is not 'A' or 'B'.
*/
void vld_validate_order(obk_order_t* buffer, int32_t count);

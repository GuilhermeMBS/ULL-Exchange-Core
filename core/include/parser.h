#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "errorlib.h"
#include "book.h"

/*
Return: pointer to the beginning of the Order array in RAM.
Error: NULL if the file does not exist or malloc fails.
*/
obk_order_t* prs_create_orders(const char* csv_path, int32_t* total_count);

/*
Return: 0 on success.
Error: -1 if the pointer is already NULL.
*/
ret_code_t prs_free_buffer(obk_order_t* buffer);

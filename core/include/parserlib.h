#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "retcodes.h"
#include "book.h"


typedef struct prs_instance_private_s* prs_handle_pt; // Opaque pointer


/**
 * @brief Parses the market CSV file and allocates the internal orders buffer.
 * @param csv_path Path to the market CSV input file.
 * @param handle Reference to the handle that will store the allocated instance pointer.
 * @param total_count Pointer that will receive the total number of processed orders.
 * @return ERR_NONE on success, or a corresponding error code if file or allocation fails.
 */
ret_code_t prs_create_orders(const char* csv_path, prs_handle_pt* handle, int32_t* total_count);


/**
 * @brief Safely fetches a copy of an order from the internal buffer by its index.
 * @param handle The active parser instance handle.
 * @param idx The targeted buffer array index position.
 * @param out_order Destination pointer where the order data will be safely copied.
 * @return ERR_NONE on success, or ERR_ORD if the index is out of bounds or handle is invalid.
 */
ret_code_t prs_get_order_by_index(prs_handle_pt handle, int32_t idx, obk_order_pt out_order);


/**
 * @brief Frees the parser instance and its internal orders buffer.
 * @param handle The parser handle instance to be destroyed.
 * @return ERR_NONE on success, or ERR_ORD if the pointer handle is already NULL.
 */
ret_code_t prs_free_buffer(prs_handle_pt handle);

#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "errorlib.h"
#include "book.h"

/**
 * @brief Valida todas as ordens do buffer, marcando como inválidas as que violam as regras.
 * @param buffer Array de ordens a ser validado.
 * @param count Número de ordens no buffer.
 * @note Define order_id como -1 nas ordens inválidas. Regras: preço ou quantidade <= 0, ou side diferente de 'A' ou 'B'.
 */
void vld_validate_order(obk_order_t* buffer, int32_t count);

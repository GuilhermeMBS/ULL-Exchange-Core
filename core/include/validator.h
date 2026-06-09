#pragma once
 
#include <stdint.h>
#include <stdlib.h>

#include "errorlib.h"
#include "book.h"

/*
 * vld_validate_order — valida todas as ordens do buffer in-place.
 *
 * Ordens já marcadas como inválidas pelo parser (linhas malformadas)
 * apenas têm seus flags normalizados — não precisam ser reavaliadas.
 *
 * Regras aplicadas a cada ordem:
 *   Regra 1: preço deve ser estritamente positivo
 *   Regra 2: quantidade deve ser >= 1
 *   Regra 3: lado deve ser 'A' (Ask) ou 'B' (Bid)
 *
 * Ordens inválidas são marcadas com is_valid = false e order_id = -1.
 *
 * Retorno: nenhum.
 */
void vld_validate_order(obk_order_t* buffer, int32_t count);
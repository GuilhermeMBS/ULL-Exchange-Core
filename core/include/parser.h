#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "errorlib.h"
#include "book.h"

/**
 * @brief Carrega e parseia o CSV, retornando o array de ordens em memória.
 * @param csv_path Caminho para o arquivo CSV.
 * @param total_count Ponteiro que receberá o número total de ordens lidas.
 * @return Ponteiro para o início do array de ordens em RAM, ou NULL se o arquivo não existir ou malloc falhar.
 */
obk_order_t* prs_create_orders(const char* csv_path, int32_t* total_count);

/**
 * @brief Libera o buffer de ordens alocado.
 * @param buffer Ponteiro para o buffer a ser liberado.
 * @return 0 em caso de sucesso, ERR_ORD se o ponteiro já for NULL.
 */
ret_code_t prs_free_buffer(obk_order_t* buffer);

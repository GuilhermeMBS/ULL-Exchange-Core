#pragma once
 
#include <stdint.h>
#include <stdlib.h>
#include "errorlib.h"
#include "book.h"

/*
 * prs_create_orders — lê o CSV, aloca o buffer contíguo
 * e valida as ordens.
 *
 * Parâmetros:
 *   csv_path    → caminho do arquivo CSV de mercado
 *   total_count → saída: número total de ordens carregadas
 *
 * Retorno:
 *   Ponteiro para o array de obk_order_t em caso de sucesso.
 *   NULL se o arquivo não existir ou se houver falha de malloc.
 */
obk_order_t* prs_create_orders(const char* csv_path, int32_t* total_count);

/*
 * prs_free_buffer — libera o buffer alocado por prs_create_orders.
 *
 * Retorno:
 *    0  → sucesso
 *   -1  → buffer já era NULL
 */
ret_code_t prs_free_buffer(obk_order_t* buffer);
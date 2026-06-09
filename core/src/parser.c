/*
 * parser.c — ULL Exchange Core
 * Módulo de ingestão de dados (Parser)
 *
 * Responsável: Gabriel
 *
 * Lê o arquivo CSV de mercado linha a linha, converte cada campo para
 * a struct obk_order_t e armazena tudo em um buffer contíguo em RAM.
 * Após o carregamento, invoca vld_validate_order() para marcar as
 * ordens inválidas antes de devolver o controle ao Matching Engine.
 *
 * Convenções do grupo:
 *   - Tipo de ordem  : obk_order_t  (definido em book.h)
 *   - Side no CSV    : 'A' = Ask (venda) | 'B' = Bid (compra)
 *   - Código de erro : ret_code_t   (definido em errorlib.h)
 *   - Ordens inválidas: is_valid = false, order_id = (uint32_t)-1
 *
 * Formato esperado do CSV (sem espaços extras, cabeçalho obrigatório):
 *   timestamp,order_id,client_id,quantity,price,symbol,side
 *   1748000001,1,42,100,150.50,PETR4,B
 *   1748000002,2,43,200,149.00,PETR4,A
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "errorlib.h"   /* ret_code_t, ERR_NONE, ERR_ORD, ERR_MEM */
#include "book.h"       /* obk_order_t, obk_order_pt, tm_stmp_t   */
#include "parser.h"     /* prs_create_orders, prs_free_buffer      */
#include "validator.h"  /* vld_validate_order                      */

/* ─────────────────────────────────────────────────────────────────────────
 * Constantes internas
 * ───────────────────────────────────────────────────────────────────────── */

#define PRS_MAX_LINE 256   /* tamanho máximo de uma linha do CSV */

/* ─────────────────────────────────────────────────────────────────────────
 * Funções auxiliares (static — sem linkagem externa)
 * ───────────────────────────────────────────────────────────────────────── */

/*
 * prs_count_lines — conta as linhas de dados do CSV (exclui cabeçalho
 * e linhas em branco) e rebobina o ponteiro para o início.
 *
 * Retorno: número de linhas de dados (>= 0), ou -1 em erro de I/O.
 */
static ret_code_t prs_count_lines(FILE *fp) {
    char    line[PRS_MAX_LINE];
    int32_t count = 0;
    int32_t first = 1;

    rewind(fp);

    while (fgets(line, sizeof(line), fp)) {
        if (first) { first = 0; continue; }           /* pula cabeçalho */
        if (line[0] == '\n' || line[0] == '\r' ||
            line[0] == '\0') continue;                 /* pula em branco */
        count++;
    }

    if (ferror(fp)) { rewind(fp); return -1; }

    rewind(fp);
    return count;
}

/*
 * prs_parse_line — converte uma linha CSV em obk_order_t.
 *
 * Formato: timestamp,order_id,client_id,quantity,price,symbol,side
 *
 * Retorno:
 *   0   → conversão OK
 *  -1   → linha malformada (campos insuficientes ou tipo errado)
 */
static ret_code_t prs_parse_line(const char *line, obk_order_t *out) {
    if (!line || !out) return -1;

    uint32_t ts, oid, cid, qty;
    double   price;
    char     symbol[8];
    char     side;

    uint32_t parsed = sscanf(line,
                        "%u,%u,%u,%u,%lf,%7[^,],%c",
                        &ts, &oid, &cid, &qty,
                        &price, symbol, &side);

    if (parsed != 7) return ERR_ORD;

    out->timestamp = (tm_stmp_t)ts;
    out->order_id  = oid;
    out->client_id = cid;
    out->quantity  = qty;
    out->price     = price;
    out->side      = side;
    out->is_valid  = true;   /* o Validator ajusta se necessário */

    strncpy(out->symbol, symbol, sizeof(out->symbol) - 1);
    out->symbol[sizeof(out->symbol) - 1] = '\0';

    return ERR_NONE;
}

/* ─────────────────────────────────────────────────────────────────────────
 * Interface pública
 * ───────────────────────────────────────────────────────────────────────── */

/*
 * prs_create_orders — lê o CSV, aloca o buffer contíguo e valida as ordens.
 *
 * Parâmetros:
 *   csv_path    → caminho do arquivo CSV de mercado
 *   total_count → saída: número total de ordens carregadas
 *
 * Retorno:
 *   Ponteiro para o array de obk_order_t em caso de sucesso.
 *   NULL se o arquivo não existir ou se houver falha de malloc.
 */
obk_order_t* prs_create_orders(const char *csv_path, int32_t *total_count) {
    if (!csv_path || !total_count) return NULL;

    *total_count = 0;

    /* ── Abre o arquivo ── */
    FILE *fp = fopen(csv_path, "r");
    if (!fp) return NULL;

    /* ── Conta as linhas para dimensionar o buffer de uma vez (sem realloc) ── */
    int32_t total = prs_count_lines(fp);
    if (total <= 0) { fclose(fp); return NULL; }

    /* ── Alocação única do buffer contíguo ── */
    obk_order_t *buffer = (obk_order_t *)malloc((size_t)total * sizeof(obk_order_t));
    if (!buffer) { fclose(fp); return NULL; }

    /* ── Percorre o CSV e preenche o buffer ── */
    char    line[PRS_MAX_LINE];
    int32_t first  = 1;
    int32_t loaded = 0;

    rewind(fp);

    while (fgets(line, sizeof(line), fp) && loaded < total) {
        if (first) { first = 0; continue; }   /* pula cabeçalho */

        /* Remove \n / \r do final */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        if (len == 0) continue;   /* ignora linhas em branco */

        obk_order_t *cur = &buffer[loaded];

        if (prs_parse_line(line, cur) != 0) {
            /*
             * Linha malformada: zera a entrada e marca como inválida
             * seguindo o contrato do Validator.
             */
            memset(cur, 0, sizeof(obk_order_t));
            cur->order_id = (uint32_t)-1;
            cur->is_valid = false;
        }

        loaded++;
    }

    fclose(fp);

    /* ── Invoca o Validator em lote sobre o buffer carregado ── */
    vld_validate_order(buffer, loaded);

    *total_count = loaded;
    return buffer;
}

/*
 * prs_free_buffer — libera o buffer alocado por prs_create_orders.
 *
 * Retorno:
 *    0  → sucesso
 *   -1  → buffer já era NULL
 */
ret_code_t prs_free_buffer(obk_order_t *buffer) {
    if (!buffer) return ERR_ORD;
    free(buffer);
    return ERR_NONE;
}

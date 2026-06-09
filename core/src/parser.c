
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "errorlib.h"
#include "book.h"
#include "parser.h"
#include "validator.h"

#define PRS_MAX_LINE 256

static ret_code_t prs_count_lines(FILE *fp) {
    char    line[PRS_MAX_LINE];
    int32_t count = 0;
    int32_t first = 1;

    rewind(fp);

    while (fgets(line, sizeof(line), fp)) {
        if (first) { first = 0; continue; }           /* skip header */
        if (line[0] == '\n' || line[0] == '\r' ||
            line[0] == '\0') continue;                 /* skip blank lines */
        count++;
    }

    if (ferror(fp)) { rewind(fp); return -1; }

    rewind(fp);
    return count;
}

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
    out->is_valid  = true;   /* validator adjusts if needed */

    strncpy(out->symbol, symbol, sizeof(out->symbol) - 1);
    out->symbol[sizeof(out->symbol) - 1] = '\0';

    return ERR_NONE;
}

obk_order_t* prs_create_orders(const char *csv_path, int32_t *total_count) {
    if (!csv_path || !total_count) return NULL;

    *total_count = 0;

    FILE *fp = fopen(csv_path, "r");
    if (!fp) return NULL;

    int32_t total = prs_count_lines(fp);
    if (total <= 0) { fclose(fp); return NULL; }

    obk_order_t *buffer = (obk_order_t *)malloc((size_t)total * sizeof(obk_order_t));
    if (!buffer) { fclose(fp); return NULL; }

    char    line[PRS_MAX_LINE];
    int32_t first  = 1;
    int32_t loaded = 0;

    rewind(fp);

    while (fgets(line, sizeof(line), fp) && loaded < total) {
        if (first) { first = 0; continue; }   /* skip header */

        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        if (len == 0) continue;

        obk_order_t *cur = &buffer[loaded];

        if (prs_parse_line(line, cur) != 0) {
            memset(cur, 0, sizeof(obk_order_t));
            cur->order_id = (uint32_t)-1;
            cur->is_valid = false;
        }

        loaded++;
    }

    fclose(fp);

    vld_validate_order(buffer, loaded);

    *total_count = loaded;
    return buffer;
}

ret_code_t prs_free_buffer(obk_order_t *buffer) {
    if (!buffer) return ERR_ORD;
    free(buffer);
    return ERR_NONE;
}

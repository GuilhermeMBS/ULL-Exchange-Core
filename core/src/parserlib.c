#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "retcodes.h"
#include "book.h"
#include "parserlib.h"
#include "validator.h"

#define PRS_MAX_LINE 256


struct prs_instance_private_s {
    obk_order_pt orders_array;
    int32_t count;
};


typedef struct prs_instance_private_s* prs_instance_private_pt;


static ret_code_t prs_count_lines(FILE *fp, int32_t *out_count) {
    char    line[PRS_MAX_LINE];
    int32_t count = 0;
    int32_t first = 1;

    if (!fp || !out_count) return ERR_ORD;

    rewind(fp);

    while (fgets(line, sizeof(line), fp)) {
        if (first) { first = 0; continue; }           /* skip header */
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue; /* skip blank lines */
        count++;
    }

    if (ferror(fp)) { 
        rewind(fp); 
        return ERR_ORD; 
    }

    rewind(fp);
    *out_count = count;
    return ERR_NONE;
}


static ret_code_t prs_parse_line(const char *line, obk_order_pt out) {
    if (!line || !out) return ERR_ORD;

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
    out->is_valid  = true;

    strncpy(out->symbol, symbol, sizeof(out->symbol) - 1);
    out->symbol[sizeof(out->symbol) - 1] = '\0';

    return ERR_NONE;
}


ret_code_t prs_create_orders(const char *csv_path, prs_handle_pt *handle, int32_t *total_count) {
    ret_code_t code;
    int32_t total = 0;

    if (!csv_path || !handle || !total_count) {
        code = ERR_ORD;
        return code;
    }

    *total_count = 0;
    *handle = NULL;

    FILE *fp = fopen(csv_path, "r");
    if (!fp) {
        code = ERR_ORD;
        return code;
    }

    code = prs_count_lines(fp, &total);
    if (code != ERR_NONE) {
        fclose(fp);
        return code;
    }

    if (total <= 0) { 
        fclose(fp); 
        code = ERR_ORD;
        return code; 
    }

    prs_instance_private_pt instance = (prs_instance_private_pt)malloc(sizeof(struct prs_instance_private_s));
    if (!instance) { 
        fclose(fp); 
        code = ERR_MEM;
        err_check_error(code);
        return code; 
    }

    instance->orders_array = (obk_order_pt)malloc((size_t)total * sizeof(obk_order_t));
    if (!instance->orders_array) { 
        fclose(fp); 
        free(instance); 
        code = ERR_MEM;
        err_check_error(code);
        return code; 
    }

    char    line[PRS_MAX_LINE];
    int32_t first  = 1;
    int32_t loaded = 0;

    rewind(fp);

    while (fgets(line, sizeof(line), fp) && loaded < total) {
        if (first) { first = 0; continue; }           /* skip header */

        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        if (len == 0) continue;

        obk_order_pt cur = &(instance->orders_array[loaded]);

        code = prs_parse_line(line, cur);
        if (code != ERR_NONE) {
            memset(cur, 0, sizeof(obk_order_t));
            cur->order_id = (uint32_t)-1;
            cur->is_valid = false;
        } else {
            bool is_order_valid = true;

            /* Standard C handles structure copy-by-value naturally upon passing it as an argument */
            vld_validate_order(*cur, &is_order_valid);
            
            if (!is_order_valid) {
                memset(cur, 0, sizeof(obk_order_t));
                cur->order_id = (uint32_t)-1;
                cur->is_valid = false;
            }
        }

        loaded++;
    }

    fclose(fp);

    instance->count = loaded;

    *total_count = loaded;
    *handle = (prs_handle_pt)instance;
    
    return ERR_NONE;
}


ret_code_t prs_get_order_by_index(prs_handle_pt handle, int32_t idx, obk_order_pt out_order) {
    ret_code_t code;

    if (handle == NULL || out_order == NULL) {
        code = ERR_ORD;
        return code;
    }

    prs_instance_private_pt instance = (prs_instance_private_pt)handle;

    if (idx < 0 || idx >= instance->count) {
        code = ERR_ORD;
        return code;
    }

    *out_order = instance->orders_array[idx];
    
    return ERR_NONE;
}


ret_code_t prs_free_buffer(prs_handle_pt handle) {
    ret_code_t code;

    if (!handle) {
        code = ERR_ORD;
        return code;
    }
    
    prs_instance_private_pt instance = (prs_instance_private_pt)handle;
    
    if (instance->orders_array) {
        free(instance->orders_array);
    }
    free(instance);
    
    return ERR_NONE;
}

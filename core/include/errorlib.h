#pragma once

#include <stdint.h>


typedef enum {
    ERR_NONE = 0,
    ERR_ORD = -1,
    ERR_MEM = -3
} ret_code_t;


void err_check_error(ret_code_t code);

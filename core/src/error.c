#pragma once

#include <stdio.h>
#include <stdint.h>

#include "error.h"


typedef int8_t ret_code_t;


void err_check_error(ret_code_t code) {
    if (code == 0) return;
    else if (code == -1) printf("\nOrder Error!\n");
    else if (code == -3) printf("\nMemory Error!\n");
    
    exit(code);
};

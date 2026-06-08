#include <stdio.h>
#include <stdlib.h>

#include "errorlib.h"


void err_check_error(ret_code_t code) {
    if (code == ERR_NONE) return;
    else if (code == ERR_ORD) printf("\nOrder Error!\n");
    else if (code == ERR_MEM) printf("\nMemory Error!\n");
    
    exit(code);
}

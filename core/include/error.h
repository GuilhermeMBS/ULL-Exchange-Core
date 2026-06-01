// Estruturas e variáveis compartilhadas por todos os arquivos C

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#define ERR_NONE 0
#define ERR_ORD -1
#define ERR_MEM -3


typedef int8_t ret_code_t;
typedef uint32_t tm_stmp_t;


void err_check_error(ret_code_t code);

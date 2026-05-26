#pragma once

#include "common.h"
#include <stdio.h>
#include <stdint.h>

void checkError(int32_t code) {
    if(code == 0) return;
};
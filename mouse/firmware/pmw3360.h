#pragma once
#include <stdint.h>
#include <stdbool.h>

bool pmw_init(void);
void pmw_burst(int16_t *dx, int16_t *dy);
void pmw_set_cpi(uint16_t cpi);

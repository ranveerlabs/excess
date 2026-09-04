#include <stdio.h>
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "pmw3360.h"

#define BTN_L  7
#define BTN_R  8
#define BTN_M  9
#define BTN_4  10
#define BTN_5  11

static const uint8_t btn_pin[5] = { BTN_L, BTN_R, BTN_M, BTN_4, BTN_5 };
static uint8_t stable, raw_last;
static uint32_t last_change;

#define DEBOUNCE_MS 8

static uint8_t read_btns(void)
{
    uint8_t r = 0;
    for (int i = 0; i < 5; i++) if (!gpio_get(btn_pin[i])) r |= (1 << i);

    uint32_t now = board_millis();
    if (r != raw_last) { raw_last = r; last_change = now; }
    else if (now - last_change >= DEBOUNCE_MS) stable = r;
    return stable;
}

int main(void)
{
    board_init();
    stdio_init_all();
    tusb_init();

    for (int i = 0; i < 5; i++) {
        gpio_init(btn_pin[i]);
        gpio_set_dir(btn_pin[i], GPIO_IN);
        gpio_pull_up(btn_pin[i]);
    }

    sleep_ms(500);
    if (!pmw_init()) {
        printf("pmw:bad\n");
    }
    pmw_set_cpi(1600);

    int16_t ax = 0, ay = 0;

    while (1) {
        tud_task();

        int16_t dx, dy;
        pmw_burst(&dx, &dy);
        ax += dx;
        ay += dy;

        if (!tud_hid_ready()) continue;

        uint8_t b = read_btns();

        int8_t sx = ax >  127 ? 127 : (ax < -127 ? -127 : ax);
        int8_t sy = ay >  127 ? 127 : (ay < -127 ? -127 : ay);
        ax -= sx;
        ay -= sy;

        // sensor is rotated 90 in the shell, sy and sx are the right way round
        if (sx || sy || b != 0) tud_hid_mouse_report(0, b, sy, sx, 0, 0);
    }
}

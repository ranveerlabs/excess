// pmw3360 over spi. mode 3, msb first, 2mhz is fine 8 is not
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pmw3360.h"
#include "srom_0x04.h"   // pixarts blob, not mine to commit. grab it from any qmk tree

#define SCK  2
#define MOSI 3
#define MISO 4
#define CS   5
#define MOT  6

#define REG_PID       0x00
#define REG_MOTION    0x02
#define REG_CPI       0x0f
#define REG_CFG2      0x10
#define REG_SROM_EN   0x13
#define REG_OBS       0x24
#define REG_SROM_ID   0x2a
#define REG_PWRUP     0x3a
#define REG_SHUT      0x3b
#define REG_BURST     0x50

static inline void cs_lo(void) { gpio_put(CS, 0); busy_wait_us(1); }
static inline void cs_hi(void) { busy_wait_us(1); gpio_put(CS, 1); }

static void wr(uint8_t r, uint8_t v)
{
    uint8_t b[2] = { r | 0x80, v };
    cs_lo();
    spi_write_blocking(spi0, b, 2);
    busy_wait_us(35);      // tSCLK-NCS for write
    cs_hi();
    busy_wait_us(180);     // tSWW. shorter and writes start dropping
}

static uint8_t rd(uint8_t r)
{
    uint8_t a = r & 0x7f, v = 0;
    cs_lo();
    spi_write_blocking(spi0, &a, 1);
    busy_wait_us(160);     // tSRAD
    spi_read_blocking(spi0, 0, &v, 1);
    busy_wait_us(1);
    cs_hi();
    busy_wait_us(19);
    return v;
}

static void srom_load(void)
{
    wr(REG_CFG2, 0x20);
    wr(REG_SROM_EN, 0x1d);
    sleep_ms(10);
    wr(REG_SROM_EN, 0x18);

    // this one is bit banged out in one long cs low, cant use wr()
    uint8_t a = REG_SROM_EN | 0x80;
    cs_lo();
    spi_write_blocking(spi0, &a, 1);
    busy_wait_us(15);
    for (unsigned i = 0; i < sizeof(srom); i++) {
        uint8_t b = srom[i];
        spi_write_blocking(spi0, &b, 1);
        busy_wait_us(15);
    }
    cs_hi();
    busy_wait_us(200);

    rd(REG_SROM_ID);
    wr(REG_CFG2, 0x00);
}

uint8_t pmw_id(void) { return rd(REG_PID); }

bool pmw_init(void)
{
    spi_init(spi0, 2 * 1000 * 1000);
    spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);
    gpio_set_function(MISO, GPIO_FUNC_SPI);
    gpio_init(CS); gpio_set_dir(CS, GPIO_OUT); gpio_put(CS, 1);
    gpio_init(MOT); gpio_set_dir(MOT, GPIO_IN); gpio_pull_up(MOT);

    cs_hi(); cs_lo(); cs_hi();
    wr(REG_SHUT, 0xb6);
    sleep_ms(300);

    cs_lo(); sleep_us(40); cs_hi(); sleep_us(40);
    wr(REG_PWRUP, 0x5a);
    sleep_ms(50);

    // dump the motion regs once or it wont start reporting
    for (int i = 0x02; i <= 0x06; i++) rd(i);

    srom_load();

    uint8_t id = rd(REG_PID);
    printf("pmw:%02x\n", id);
    return id == 0x42;
}

void pmw_set_cpi(uint16_t cpi)
{
    // 100 steps. 0 is 100cpi
    uint8_t v = (cpi / 100) - 1;
    wr(REG_CPI, v);
}

void pmw_burst(int16_t *dx, int16_t *dy)
{
    uint8_t b[6];
    wr(REG_BURST, 0x00);
    uint8_t a = REG_BURST;
    cs_lo();
    spi_write_blocking(spi0, &a, 1);
    busy_wait_us(35);
    spi_read_blocking(spi0, 0, b, 6);
    cs_hi();
    busy_wait_us(1);

    *dx = (int16_t)((b[3] << 8) | b[2]);
    *dy = (int16_t)((b[5] << 8) | b[4]);
    // b[0] motion, b[1] observation, squal is b[6] if you read 7
}

#pragma once
#include <stdint.h>

// 64 subcarriers for HT20, only the middle ones carry anything
#define CSI_SUBC      64
#define CSI_LO        6
#define CSI_HI        58

typedef struct {
    float amp[CSI_SUBC];
    int8_t rssi;
    uint32_t t_ms;
} csi_frame_t;

void csi_rx_start(void);

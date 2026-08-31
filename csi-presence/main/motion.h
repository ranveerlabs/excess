#pragma once
#include "csi_rx.h"

void motion_feed(const csi_frame_t *f);
int  motion_state(void);   // 1 = someone there
float motion_score(void);

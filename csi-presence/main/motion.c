#include <math.h>
#include <string.h>
#include "esp_log.h"
#include "motion.h"

#define W 32

static float base[CSI_SUBC];
static int   have_base;
static float ring[W];
static int   rp;
static int   filled;

static float score;
static int   present;
static int   hold;

static const float TH_ON  = 1.8f;
static const float TH_OFF = 1.1f;
static const int   HOLD_N = 90;

static const char *T = "mot";

void motion_feed(const csi_frame_t *f)
{
    if (!have_base) {
        memcpy(base, f->amp, sizeof(base));
        have_base = 1;
        ESP_LOGI(T, "base:set");
        return;
    }

    float d = 0, mag = 0;
    for (int i = CSI_LO; i < CSI_HI; i++) {
        d   += fabsf(f->amp[i] - base[i]);
        mag += base[i];
    }
    d = d / (mag + 1e-6f) * 100.0f;

    ring[rp] = d;
    rp = (rp + 1) % W;
    if (filled < W) filled++;

    float m = 0;
    for (int i = 0; i < filled; i++) m += ring[i];
    m /= filled;

    float v = 0;
    for (int i = 0; i < filled; i++) { float e = ring[i] - m; v += e * e; }
    v = sqrtf(v / filled);

    score = v;

    if (!present && score > TH_ON) {
        present = 1;
        hold = HOLD_N;
        ESP_LOGI(T, "in %.2f", score);
    } else if (present) {
        if (score > TH_OFF) hold = HOLD_N;
        else if (--hold <= 0) {
            present = 0;
            ESP_LOGI(T, "out");
        }
    }

    if (!present) {
        for (int i = CSI_LO; i < CSI_HI; i++)
            base[i] = base[i] * 0.999f + f->amp[i] * 0.001f;
    }
}

int motion_state(void) { return present; }
float motion_score(void) { return score; }

#include <math.h>
#include <string.h>
#include "esp_log.h"
#include "motion.h"

// slow baseline of what the room looks like empty, then how far each
// frame sits off it

static float base[CSI_SUBC];
static int   have_base;
static float score;
static int   present;

static const float TH = 0.995f;

static const char *T = "mot";

static float corr(const float *a, const float *b)
{
    float num = 0, da = 0, db = 0;
    for (int i = CSI_LO; i < CSI_HI; i++) {
        num += a[i] * b[i];
        da  += a[i] * a[i];
        db  += b[i] * b[i];
    }
    return num / (sqrtf(da) * sqrtf(db) + 1e-6f);
}

void motion_feed(const csi_frame_t *f)
{
    if (!have_base) {
        memcpy(base, f->amp, sizeof(base));
        have_base = 1;
        ESP_LOGI(T, "base:set");
        return;
    }

    score = corr(base, f->amp);
    int p = score < TH;
    if (p != present) {
        present = p;
        ESP_LOGI(T, "%s %.4f", p ? "in" : "out", score);
    }
}

int motion_state(void) { return present; }
float motion_score(void) { return score; }

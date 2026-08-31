#include <math.h>
#include <string.h>
#include "esp_log.h"
#include "motion.h"

// slow baseline of what the room looks like empty, then how far each
// frame sits off it

#define W 32          // ring of frames the variance runs over

static float base[CSI_SUBC];
static int   have_base;
static float ring[W];
static int   rp;
static int   filled;

static float score;
static int   present;
static int   hold;    // frames left before we let go

// tuned sat on the floor of my room with the esp on the shelf, dont
// expect it to carry to another room
static const float TH_ON  = 1.8f;
static const float TH_OFF = 1.1f;
static const int   HOLD_N = 90;

static const char *T = "mot";

#if 0
// first pass was per subcarrier correlation against the baseline vector.
// worked, but it also fired every time the AP shifted rate so
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
#endif

void motion_feed(const csi_frame_t *f)
{
    if (!have_base) {
        memcpy(base, f->amp, sizeof(base));
        have_base = 1;
        ESP_LOGI(T, "base:set");
        return;
    }

    // l1 distance off the baseline, normalised by how much signal there is
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

    // creep the baseline when nobodys there. a door left open or the
    // heating coming on both walk the amplitudes and it never comes back
    // otherwise
    if (!present) {
        for (int i = CSI_LO; i < CSI_HI; i++)
            base[i] = base[i] * 0.999f + f->amp[i] * 0.001f;
    }
}

int motion_state(void) { return present; }
float motion_score(void) { return score; }

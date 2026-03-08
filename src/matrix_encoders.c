#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/keymap.h>

#define NUM_ENCODERS 2
#define ENC_SYNTH_SOURCE 1  // synthetic re-injected position events for encoders

// logical position 番号
#define RE1_A_POS 14
#define RE1_B_POS 15
#define RE2_A_POS 62
#define RE2_B_POS 63

static const uint8_t a_pos[NUM_ENCODERS] = { RE1_A_POS, RE2_A_POS };
static const uint8_t b_pos[NUM_ENCODERS] = { RE1_B_POS, RE2_B_POS };

static uint8_t ab_now[NUM_ENCODERS];   // bit0=A, bit1=B
static uint8_t ab_prev[NUM_ENCODERS];
static int8_t  acc[NUM_ENCODERS];

// Grayコード遷移テーブル
static int8_t quad_dir(uint8_t prev, uint8_t now) {
    static const int8_t tbl[4][4] = {
        /*to: 00  01  10  11 */
        /*00*/ { 0, +1, -1,  0},
        /*01*/ {-1,  0,  0, +1},
        /*10*/ {+1,  0,  0, -1},
        /*11*/ { 0, -1, +1,  0},
    };
    return tbl[prev & 3][now & 3];
}

static void tap_position(uint32_t position) {
    int64_t ts = k_uptime_get();

    zmk_keymap_position_state_changed(
        ENC_SYNTH_SOURCE, position, true, ts
    );
    zmk_keymap_position_state_changed(
        ENC_SYNTH_SOURCE, position, false, ts
    );
}

static void on_step(int enc_index, bool clockwise) {
    if (enc_index == 0) {
        tap_position(clockwise ? RE1_A_POS : RE1_B_POS);
    } else {
        tap_position(clockwise ? RE2_A_POS : RE2_B_POS);
    }
}

static void update_ab(int enc, bool is_a, bool pressed) {
    uint8_t bit = is_a ? 0 : 1;

    ab_prev[enc] = ab_now[enc];
    if (pressed) ab_now[enc] |=  (1u << bit);
    else         ab_now[enc] &= ~(1u << bit);

    int8_t d = quad_dir(ab_prev[enc], ab_now[enc]);
    if (!d) return;

    acc[enc] += d;

        // 4 遷移 = 1 ノッチ
    if (acc[enc] >= 4) {
        acc[enc] = 0;
        on_step(enc, true);
    } else if (acc[enc] <= -4) {
        acc[enc] = 0;
        on_step(enc, false);
    }
}

static int matrix_encoders_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
    if (!ev) return ZMK_EV_EVENT_BUBBLE;

    /* 自分が再発行した synthetic event は無視して ZMK 本体へ流す */
    if (ev->source == ENC_SYNTH_SOURCE) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint32_t pos = ev->position;
    bool pressed = ev->state;

    for (int i = 0; i < NUM_ENCODERS; i++) {
        if (pos == a_pos[i]) {
            update_ab(i, true, pressed);
            return ZMK_EV_EVENT_HANDLED;
        }
        if (pos == b_pos[i]) {
            update_ab(i, false, pressed);
            return ZMK_EV_EVENT_HANDLED;
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(matrix_encoders, matrix_encoders_listener);
ZMK_SUBSCRIPTION(matrix_encoders, zmk_position_state_changed);
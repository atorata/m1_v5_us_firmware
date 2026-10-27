// Copyright 2026 atorata
// SPDX-License-Identifier: GPL-2.0-or-later
//
// VialRGB lighting protocol, so OpenRGB (1.0 or newer, "QMK VialRGB Device")
// can control the LEDs. It rides on the VIA raw HID channel: same 32-byte
// packets, command ids VIA does not use (0xFE) or channel ids above VIA's
// (0x40+ on custom get/set value). VIA keeps working as before.
//
// Protocol: https://github.com/vial-kb/vial-qmk (quantum/vialrgb.c).
// Only what OpenRGB needs is implemented; this is not a Vial keyboard.

#include QMK_KEYBOARD_H
#include <string.h>
#include "raw_hid.h"
#include "via.h"

#define VIAL_PROTOCOL_VERSION    6
#define VIALRGB_PROTOCOL_VERSION 1
#define VIALRGB_PACKET_SIZE      32

enum {
    vial_command         = 0xFE,
    vial_get_keyboard_id = 0x00,
};

enum {
    vialrgb_get_info        = 0x40,
    vialrgb_get_mode        = 0x41,
    vialrgb_get_supported   = 0x42,
    vialrgb_get_number_leds = 0x43,
    vialrgb_get_led_info    = 0x44,
};

enum {
    vialrgb_set_mode       = 0x41,
    vialrgb_direct_fastset = 0x42,
};

enum {
    VIALRGB_EFFECT_OFF    = 0,
    VIALRGB_EFFECT_DIRECT = 1,
};

typedef struct {
    uint16_t vialrgb_id;
    uint16_t qmk_id;
} vialrgb_mode_t;

// VialRGB ids are a fixed list (never reordered upstream); map the ones this
// keyboard has. Effects not on the list (pixel flow, RGB recording) report Off.
static const vialrgb_mode_t vialrgb_modes[] = {
    {VIALRGB_EFFECT_DIRECT, RGB_MATRIX_CUSTOM_OPENRGB_DIRECT},
    { 2, RGB_MATRIX_SOLID_COLOR},
#ifdef ENABLE_RGB_MATRIX_ALPHAS_MODS
    { 3, RGB_MATRIX_ALPHAS_MODS},
#endif
#ifdef ENABLE_RGB_MATRIX_GRADIENT_UP_DOWN
    { 4, RGB_MATRIX_GRADIENT_UP_DOWN},
#endif
#ifdef ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
    { 5, RGB_MATRIX_GRADIENT_LEFT_RIGHT},
#endif
#ifdef ENABLE_RGB_MATRIX_BREATHING
    { 6, RGB_MATRIX_BREATHING},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_SAT
    { 7, RGB_MATRIX_BAND_SAT},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_VAL
    { 8, RGB_MATRIX_BAND_VAL},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_PINWHEEL_SAT
    { 9, RGB_MATRIX_BAND_PINWHEEL_SAT},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_PINWHEEL_VAL
    {10, RGB_MATRIX_BAND_PINWHEEL_VAL},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_SPIRAL_SAT
    {11, RGB_MATRIX_BAND_SPIRAL_SAT},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_SPIRAL_VAL
    {12, RGB_MATRIX_BAND_SPIRAL_VAL},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_ALL
    {13, RGB_MATRIX_CYCLE_ALL},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
    {14, RGB_MATRIX_CYCLE_LEFT_RIGHT},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_UP_DOWN
    {15, RGB_MATRIX_CYCLE_UP_DOWN},
#endif
#ifdef ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
    {16, RGB_MATRIX_RAINBOW_MOVING_CHEVRON},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_OUT_IN
    {17, RGB_MATRIX_CYCLE_OUT_IN},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_OUT_IN_DUAL
    {18, RGB_MATRIX_CYCLE_OUT_IN_DUAL},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
    {19, RGB_MATRIX_CYCLE_PINWHEEL},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_SPIRAL
    {20, RGB_MATRIX_CYCLE_SPIRAL},
#endif
#ifdef ENABLE_RGB_MATRIX_DUAL_BEACON
    {21, RGB_MATRIX_DUAL_BEACON},
#endif
#ifdef ENABLE_RGB_MATRIX_RAINBOW_BEACON
    {22, RGB_MATRIX_RAINBOW_BEACON},
#endif
#ifdef ENABLE_RGB_MATRIX_RAINBOW_PINWHEELS
    {23, RGB_MATRIX_RAINBOW_PINWHEELS},
#endif
#ifdef ENABLE_RGB_MATRIX_RAINDROPS
    {24, RGB_MATRIX_RAINDROPS},
#endif
#ifdef ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
    {25, RGB_MATRIX_JELLYBEAN_RAINDROPS},
#endif
#ifdef ENABLE_RGB_MATRIX_HUE_BREATHING
    {26, RGB_MATRIX_HUE_BREATHING},
#endif
#ifdef ENABLE_RGB_MATRIX_HUE_PENDULUM
    {27, RGB_MATRIX_HUE_PENDULUM},
#endif
#ifdef ENABLE_RGB_MATRIX_HUE_WAVE
    {28, RGB_MATRIX_HUE_WAVE},
#endif
#ifdef ENABLE_RGB_MATRIX_TYPING_HEATMAP
    {29, RGB_MATRIX_TYPING_HEATMAP},
#endif
#ifdef ENABLE_RGB_MATRIX_DIGITAL_RAIN
    {30, RGB_MATRIX_DIGITAL_RAIN},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
    {31, RGB_MATRIX_SOLID_REACTIVE_SIMPLE},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE
    {32, RGB_MATRIX_SOLID_REACTIVE},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
    {33, RGB_MATRIX_SOLID_REACTIVE_WIDE},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE
    {34, RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_CROSS
    {35, RGB_MATRIX_SOLID_REACTIVE_CROSS},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTICROSS
    {36, RGB_MATRIX_SOLID_REACTIVE_MULTICROSS},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_NEXUS
    {37, RGB_MATRIX_SOLID_REACTIVE_NEXUS},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS
    {38, RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS},
#endif
#ifdef ENABLE_RGB_MATRIX_SPLASH
    {39, RGB_MATRIX_SPLASH},
#endif
#ifdef ENABLE_RGB_MATRIX_MULTISPLASH
    {40, RGB_MATRIX_MULTISPLASH},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_SPLASH
    {41, RGB_MATRIX_SOLID_SPLASH},
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
    {42, RGB_MATRIX_SOLID_MULTISPLASH},
#endif
#ifdef ENABLE_RGB_MATRIX_PIXEL_RAIN
    {43, RGB_MATRIX_PIXEL_RAIN},
#endif
#ifdef ENABLE_RGB_MATRIX_PIXEL_FRACTAL
    {44, RGB_MATRIX_PIXEL_FRACTAL},
#endif
};

#define VIALRGB_MODE_COUNT (sizeof(vialrgb_modes) / sizeof(vialrgb_modes[0]))

// Colours OpenRGB paints in Direct mode (OPENRGB_DIRECT in rgb_matrix_user.inc).
extern HSV g_openrgb_direct[RGB_MATRIX_LED_COUNT];

static uint16_t qmk_to_vialrgb(uint8_t mode) {
    for (uint8_t i = 0; i < VIALRGB_MODE_COUNT; i++) {
        if (vialrgb_modes[i].qmk_id == mode) return vialrgb_modes[i].vialrgb_id;
    }
    return VIALRGB_EFFECT_OFF;
}

static bool vialrgb_to_qmk(uint16_t id, uint8_t *mode) {
    for (uint8_t i = 0; i < VIALRGB_MODE_COUNT; i++) {
        if (vialrgb_modes[i].vialrgb_id == id) {
            *mode = vialrgb_modes[i].qmk_id;
            return true;
        }
    }
    return false;
}

static void put_u16(uint8_t *p, uint16_t v) {
    p[0] = v & 0xFF;
    p[1] = v >> 8;
}

static uint16_t get_u16(const uint8_t *p) {
    return p[0] | (p[1] << 8);
}

// args: in = largest id already received; out = next ids, padded with 0xFFFF.
static void get_supported(uint8_t *args, uint8_t length) {
    uint16_t after = get_u16(args);
    memset(args, 0xFF, length);
    for (uint8_t i = 0; i < VIALRGB_MODE_COUNT && length >= 2; i++) {
        if (vialrgb_modes[i].vialrgb_id > after) {
            put_u16(args, vialrgb_modes[i].vialrgb_id);
            args += 2;
            length -= 2;
        }
    }
}

static void get_led_info(uint8_t *args) {
    uint16_t led = get_u16(args);
    if (led >= RGB_MATRIX_LED_COUNT) return;

    args[0] = g_led_config.point[led].x;
    args[1] = g_led_config.point[led].y;
    args[2] = g_led_config.flags[led];
    args[3] = args[4] = 0xFF; // not a key
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (g_led_config.matrix_co[row][col] == led) {
                args[3] = row;
                args[4] = col;
                return;
            }
        }
    }
}

static void set_mode(const uint8_t *args) {
    uint16_t id = get_u16(args);
    uint8_t  mode;

    // Not saved to EEPROM: unplugging, or picking an effect on the keyboard,
    // brings back the keyboard's own lighting.
    if (id == VIALRGB_EFFECT_OFF) {
        rgb_matrix_disable_noeeprom();
    } else if (vialrgb_to_qmk(id, &mode)) {
        if (mode == RGB_MATRIX_CUSTOM_OPENRGB_DIRECT && rgb_matrix_get_mode() != mode) {
            memset(g_openrgb_direct, 0, sizeof(g_openrgb_direct));
        }
        rgb_matrix_enable_noeeprom();
        rgb_matrix_mode_noeeprom(mode);
    } else {
        return;
    }
    rgb_matrix_set_speed_noeeprom(args[2]);
    rgb_matrix_sethsv_noeeprom(args[3], args[4], args[5]);
}

// args: first LED (u16), LED count, then H, S, V per LED (up to 9 per packet).
static void direct_fastset(const uint8_t *args, uint8_t length) {
    uint16_t first = get_u16(args);
    uint8_t  count = args[2];

    if (3 + count * 3 > length) return;
    args += 3;
    for (uint8_t i = 0; i < count && first + i < RGB_MATRIX_LED_COUNT; i++) {
        uint8_t v = args[i * 3 + 2];
        g_openrgb_direct[first + i] = (HSV){
            .h = args[i * 3 + 0],
            .s = args[i * 3 + 1],
            .v = v > RGB_MATRIX_MAXIMUM_BRIGHTNESS ? RGB_MATRIX_MAXIMUM_BRIGHTNESS : v,
        };
    }
}

// Runs before VIA's own handling. Returns true only for VialRGB packets,
// after replying; everything else goes to VIA untouched.
bool via_command_kb(uint8_t *data, uint8_t length) {
    if (length != VIALRGB_PACKET_SIZE) return false;

    uint8_t *args = &data[2];

    switch (data[0]) {
        case vial_command:
            if (data[1] != vial_get_keyboard_id) return false;
            // Vial protocol version (u32), keyboard UID (8 bytes), flags: bit 0 = VialRGB
            memset(data, 0, length);
            data[0]  = VIAL_PROTOCOL_VERSION;
            data[12] = 1;
            break;

        case id_custom_get_value:
            switch (data[1]) {
                case vialrgb_get_info:
                    put_u16(&args[0], VIALRGB_PROTOCOL_VERSION);
                    args[2] = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
                    break;
                case vialrgb_get_mode:
                    put_u16(&args[0], rgb_matrix_is_enabled() ? qmk_to_vialrgb(rgb_matrix_get_mode()) : VIALRGB_EFFECT_OFF);
                    args[2] = rgb_matrix_get_speed();
                    args[3] = rgb_matrix_get_hue();
                    args[4] = rgb_matrix_get_sat();
                    args[5] = rgb_matrix_get_val();
                    break;
                case vialrgb_get_supported:
                    get_supported(args, length - 2);
                    break;
                case vialrgb_get_number_leds:
                    put_u16(&args[0], RGB_MATRIX_LED_COUNT);
                    break;
                case vialrgb_get_led_info:
                    get_led_info(args);
                    break;
                default:
                    return false;
            }
            break;

        case id_custom_set_value:
            switch (data[1]) {
                case vialrgb_set_mode:
                    set_mode(args);
                    break;
                case vialrgb_direct_fastset:
                    direct_fastset(args, length - 2);
                    break;
                default:
                    return false;
            }
            break;

        default:
            return false;
    }

    raw_hid_send(data, length);
    return true;
}

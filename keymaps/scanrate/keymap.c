// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Measurement build: the default keymap plus Fn + S, which types the matrix
// scan rate of the last second, e.g. "scan 1834". Build with KEYMAP=scanrate.

#include "../default/keymap.c"

// Position of S, matched by matrix position so a VIA remap does not hide it.
#define SCANRATE_ROW 3
#define SCANRATE_COL 2

static void send_scan_rate(void) {
    char buf[16];
    uint8_t i = sizeof(buf) - 1;
    uint32_t rate = get_matrix_scan_rate();

    buf[i] = '\0';
    do {
        buf[--i] = '0' + rate % 10;
        rate /= 10;
    } while (rate && i > 0);

    send_string("scan ");
    send_string(&buf[i]);
    send_string("\n");
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.key.row == SCANRATE_ROW && record->event.key.col == SCANRATE_COL && (layer_state_is(_FL) || layer_state_is(_MFL))) {
        if (record->event.pressed) {
            send_scan_rate();
        }
        return false;
    }
    return true;
}

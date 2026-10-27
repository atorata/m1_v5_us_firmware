#!/usr/bin/env bash
# One-shot build for the M1 V5 US firmware. See readme.md "Building".
#
#   ./build.sh            build with Docker (recommended)
#   ./build.sh --native   build with a locally installed QMK CLI
#
# Environment:
#   QMK_DIR   where the MonsGeek QMK tree lives (default: ~/qmk-monsgeek)
#   KEYMAP    keymap to build (default: default)
set -euo pipefail

FW="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"   # this repo
QMK="${QMK_DIR:-$HOME/qmk-monsgeek}"
KEYMAP="${KEYMAP:-default}"
KB=monsgeek/m1_v5/m1_v5_us
MODE="${1:-docker}"

case "$MODE" in
    docker|--docker) MODE=docker ;;
    --native)        MODE=native ;;
    -h|--help)       sed -n '2,9p' "$0"; exit 0 ;;
    *)               echo "unknown option: $MODE (use --native or nothing)" >&2; exit 2 ;;
esac

need() { command -v "$1" >/dev/null 2>&1 || { echo "missing tool: $1" >&2; exit 1; }; }
need git
if [ "$MODE" = docker ]; then need docker; else need qmk; fi

# 1. MonsGeek QMK fork, wireless branch, shallow (first run only)
if [ ! -d "$QMK/.git" ]; then
    echo ">> cloning MonsGeek/qmk_firmware (wireless) into $QMK"
    git clone --depth 1 --branch wireless --single-branch \
        https://github.com/MonsGeek/qmk_firmware.git "$QMK"
fi

# 2. Only the submodules this keyboard needs
echo ">> updating submodules (chibios, chibios-contrib, printf)"
git -C "$QMK" submodule update --init --depth 1 lib/chibios lib/chibios-contrib lib/printf

# 3. Reset the stock keyboard folder, then copy this repo over it
echo ">> copying $FW -> keyboards/$KB"
git -C "$QMK" checkout -q -- "keyboards/$KB"
tar -C "$FW" --exclude=.git --exclude=build.sh -cf - . | tar -C "$QMK/keyboards/$KB" -xf -

# 4. Compile
echo ">> compiling ($MODE, keymap: $KEYMAP)"
if [ "$MODE" = docker ]; then
    docker run --rm --user "$(id -u):$(id -g)" \
        -v "$QMK":/qmk_firmware -w /qmk_firmware \
        ghcr.io/qmk/qmk_cli \
        qmk compile -kb "$KB" -km "$KEYMAP"
else
    (cd "$QMK" && qmk compile -kb "$KB" -km "$KEYMAP")
fi

OUT="$QMK/monsgeek_m1_v5_m1_v5_us_${KEYMAP}.bin"
echo ">> done: $OUT"
ls -l "$OUT"

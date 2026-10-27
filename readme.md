# MonsGeek / AKKO M1 V5 (US) — custom QMK firmware

Modified firmware for the M1 V5 US (VIA, wired + Bluetooth + 2.4G).

Changes compared to the stock MonsGeek firmware:

- **Faster multi-key response:** per-key eager debounce (`sym_eager_pk`) instead of QMK's global deferred default. Each key press is sent immediately, and other keys no longer delay it.
- **Brighter RGB:** maximum brightness raised from 104 to 200.
- **Faster RGB transitions:** no blink on RGB speed, brightness, hue or mode changes, and lighter RGB processing for quicker key response.
- **Mouse keys** enabled in the default keymap.

This repository holds **only the keyboard folder**. It does not build on its own: it needs MonsGeek's QMK fork, because the wireless driver (`keyboards/linker/wireless`) is only there.

---

## Building

### What you need

| | Docker way (recommended) | Native way |
|---|---|---|
| OS | Linux, macOS, or Windows with WSL2 | Linux, macOS, or Windows with MSYS2/WSL2 |
| Tools | `git`, `rsync`, Docker | `git`, `rsync`, [QMK CLI](https://docs.qmk.fm/newbs_getting_started) (installs the `arm-none-eabi` toolchain) |
| Disk | ~1 GB (QMK image) + ~200 MB (source) | ~200 MB (source) + toolchain |
| Network | Needed for the first build only | Needed for the first build only |

The build has two sources:

- **QMK tree:** [`MonsGeek/qmk_firmware`](https://github.com/MonsGeek/qmk_firmware), branch **`wireless`**. Upstream QMK will **not** work.
- **This repo:** copied over `keyboards/monsgeek/m1_v5/m1_v5_us` in that tree.

Only 3 of the QMK submodules are needed: `lib/chibios`, `lib/chibios-contrib` and `lib/printf`. Fetching only these takes about 20 seconds. A full `qmk setup` downloads all of them (~1 GB).

### One-shot build (Docker)

Save this as `build.sh` next to your clone of this repo and run `./build.sh path/to/m1_v5_us_firmware`:

```bash
#!/usr/bin/env bash
set -euo pipefail

FW="$(cd "${1:-.}" && pwd)"                 # this repo
QMK="${QMK_DIR:-$HOME/qmk-monsgeek}"        # MonsGeek QMK tree (created on first run)
KB=monsgeek/m1_v5/m1_v5_us

# 1. MonsGeek QMK fork, wireless branch, shallow (first run only)
[ -d "$QMK/.git" ] || git clone --depth 1 --branch wireless --single-branch \
    https://github.com/MonsGeek/qmk_firmware.git "$QMK"

# 2. Only the submodules this keyboard needs
git -C "$QMK" submodule update --init --depth 1 lib/chibios lib/chibios-contrib lib/printf

# 3. Reset the stock keyboard folder, then copy this repo over it
git -C "$QMK" checkout -q -- "keyboards/$KB"
rsync -a --exclude=.git "$FW/" "$QMK/keyboards/$KB/"

# 4. Compile inside the official QMK image (no local toolchain needed)
docker run --rm --user "$(id -u):$(id -g)" \
    -v "$QMK":/qmk_firmware -w /qmk_firmware \
    ghcr.io/qmk/qmk_cli \
    qmk compile -kb "$KB" -km default

ls -l "$QMK"/monsgeek_m1_v5_m1_v5_us_default.bin
```

The first run takes a minute or two (clone, submodules and image pull). After that, a rebuild only copies the files and compiles.

The output is `~/qmk-monsgeek/monsgeek_m1_v5_m1_v5_us_default.bin`.

### Native build (no Docker)

```bash
python3 -m pip install --user qmk           # or follow the QMK getting-started guide for your OS
git clone --depth 1 --branch wireless --single-branch \
    https://github.com/MonsGeek/qmk_firmware.git ~/qmk-monsgeek
cd ~/qmk-monsgeek
git submodule update --init --depth 1 lib/chibios lib/chibios-contrib lib/printf
rsync -a --exclude=.git /path/to/m1_v5_us_firmware/ keyboards/monsgeek/m1_v5/m1_v5_us/
qmk compile -kb monsgeek/m1_v5/m1_v5_us -km default
```

If `qmk compile` complains about a missing compiler, run `qmk doctor`. It offers to install `arm-none-eabi-gcc`.

To use your own keymap, copy `keymaps/default` to `keymaps/<name>` and build with `-km <name>`.

---

## Flashing

The MCU is a **WB32** and the bootloader is `wb32-dfu`. Flash over a **USB cable**; wireless modes cannot flash.

1. **Enter the bootloader** (any of these):
   - **Fn + Right Shift + Esc** with the keyboard plugged in. ⚠ This also resets settings (VIA keymap, RGB).
   - Hold **Esc** while plugging in the cable (bootmagic). ⚠ This also resets settings.
   - Press the **reset button** on the PCB.
2. **Flash the `.bin`** (any of these):
   - [QMK Toolbox](https://github.com/qmk/qmk_toolbox/releases): open the `.bin` and click *Flash*.
   - Command line, from the QMK tree: `qmk flash -kb monsgeek/m1_v5/m1_v5_us -km default`. This needs `wb32-dfu-updater_cli`; `qmk doctor` / `qmk setup` install it.
3. Unplug and replug. Because entering the bootloader resets settings, reapply your VIA keymap and RGB settings.

---

## Troubleshooting

| Error | Cause / fix |
|---|---|
| `keyboards/linker/wireless/wireless.mk: No such file` | You are using upstream QMK. Use `MonsGeek/qmk_firmware`, branch `wireless`. |
| `undefined reference to 'im_test_rate_flag'` | Old copy of `m1_v5_us.c`. Pull the latest version of this repo; newer MonsGeek branches need this variable. |
| `lib/chibios/... No such file` / `ChibiOS not found` | Submodules missing. Run the `git submodule update ...` line above. |
| `arm-none-eabi-gcc: not found` | No toolchain. Use the Docker way, or run `qmk doctor`. |
| Docker: `permission denied` on output files | Remove `--user ...` from the `docker run` line, or `chown` the QMK tree back to yourself. |
| Keyboard not detected for flashing | It is not in the bootloader, or it is in BT/2.4G mode. Use the cable and enter the bootloader again. |

---

## Credits

- Original keyboard support: [yangzheng20003](https://github.com/yangzheng20003) / MonsGeek.
- QMK: [qmk.fm](https://qmk.fm). Licensed GPL-2.0-or-later, like the original files.

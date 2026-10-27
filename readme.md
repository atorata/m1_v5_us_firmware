# MonsGeek / AKKO M1 V5 (US) — custom QMK firmware

**English** | [Türkçe](readme.tr.md)

Modified firmware for the M1 V5 US (VIA, wired + Bluetooth + 2.4G).

Changes compared to the stock MonsGeek firmware:

- **Faster multi-key response:** per-key eager debounce (`sym_eager_pk`) instead of QMK's global deferred default. Each key press is sent immediately, and other keys no longer delay it.
- **Brighter RGB:** maximum brightness raised from 104 to 200.
- **Faster RGB transitions:** no blink on RGB speed, brightness, hue or mode changes, and lighter RGB processing for quicker key response.
- **Mouse keys** enabled in the default keymap.

This repository holds **only the keyboard folder**. It does not build on its own: it needs MonsGeek's QMK fork, because the wireless driver (`keyboards/linker/wireless`) is only there.

---

## Download (no build needed)

Ready-to-flash firmware is on the [**Releases**](https://github.com/atorata/m1_v5_us_firmware/releases) page.

1. Open the latest release and download `m1_v5_us_<version>.bin`.
2. Optionally, check it against the `.sha256` file next to it: `sha256sum -c m1_v5_us_<version>.bin.sha256`.
3. Flash it as described in [Flashing](#flashing).

Every release is built automatically by GitHub Actions from this repo, using the same `build.sh` described below. Builds for unreleased commits are attached as a `firmware` artifact to each run on the [Actions](https://github.com/atorata/m1_v5_us_firmware/actions) page; you need to be signed in to GitHub to download those.

---

## Building

### What you need

| | Docker way (recommended) | Native way |
|---|---|---|
| OS | Linux, macOS, or Windows with WSL2 | Linux, macOS, or Windows with MSYS2/WSL2 |
| Tools | `git`, `tar`, Docker | `git`, `tar`, [QMK CLI](https://docs.qmk.fm/newbs_getting_started) (installs the `arm-none-eabi` toolchain) |
| Disk | ~1 GB (QMK image) + ~200 MB (source) | ~200 MB (source) + toolchain |
| Network | Needed for the first build only | Needed for the first build only |

The build has two sources:

- **QMK tree:** [`MonsGeek/qmk_firmware`](https://github.com/MonsGeek/qmk_firmware), branch **`wireless`**. Upstream QMK will **not** work.
- **This repo:** copied over `keyboards/monsgeek/m1_v5/m1_v5_us` in that tree.

Only 4 of the QMK submodules are needed: `lib/chibios`, `lib/chibios-contrib`, `lib/printf` and `lib/lufa` (ChibiOS's USB stack uses LUFA's headers). Fetching only these takes about 20 seconds. A full `qmk setup` downloads all of them (~1 GB).

### One-shot build (`build.sh`)

The repo ships a [`build.sh`](build.sh) script that does everything in one go:

```bash
git clone https://github.com/atorata/m1_v5_us_firmware.git
cd m1_v5_us_firmware
./build.sh            # Docker (recommended): no local toolchain needed
./build.sh --native   # use your locally installed QMK CLI instead
```

The script runs four steps:
1. Clones MonsGeek's QMK fork, `wireless` branch, shallow, into `~/qmk-monsgeek`. This happens on the first run only.
2. Fetches the 4 submodules listed above.
3. Resets the stock keyboard folder, then copies this repo over it.
4. Compiles, either inside the official `ghcr.io/qmk/qmk_cli` image or with your local `qmk`.

The first run takes a minute or two (clone, submodules and image pull). After that, a rebuild only copies the files and compiles.

The output is `~/qmk-monsgeek/monsgeek_m1_v5_m1_v5_us_default.bin`.

Two environment variables change the defaults:
- `QMK_DIR`: where the QMK tree lives.
- `KEYMAP`: which keymap to build.

Example: `KEYMAP=mine QMK_DIR=~/src/qmk ./build.sh`.

### Manual steps (no script)

```bash
python3 -m pip install --user qmk           # or follow the QMK getting-started guide for your OS
git clone --depth 1 --branch wireless --single-branch \
    https://github.com/MonsGeek/qmk_firmware.git ~/qmk-monsgeek
cd ~/qmk-monsgeek
git submodule update --init --depth 1 lib/chibios lib/chibios-contrib lib/printf lib/lufa
tar -C /path/to/m1_v5_us_firmware --exclude=.git --exclude=.github --exclude=build.sh -cf - . | tar -C keyboards/monsgeek/m1_v5/m1_v5_us -xf -
qmk compile -kb monsgeek/m1_v5/m1_v5_us -km default
```

If `qmk compile` complains about a missing compiler, run `qmk doctor`. It offers to install `arm-none-eabi-gcc`.

To use your own keymap, copy `keymaps/default` to `keymaps/<name>` and build with `-km <name>`.

### Windows (WSL2)

`build.sh` works in WSL2 (for example Ubuntu). Keep these in mind:

- **Clone inside the Linux file system** (`~/...`), not under `/mnt/c/...`. On `/mnt/c` the build is much slower, and file permissions and line endings cause problems.
- **Docker:** either install Docker Desktop and enable *Settings → Resources → WSL integration* for your distro, or install Docker Engine inside WSL. Check with `docker run --rm hello-world`.
- **Without Docker:** `./build.sh --native` also works in WSL once the QMK CLI is installed there (`qmk doctor` installs the toolchain).
- **Flashing:** WSL does not see USB devices by default. The simplest way is to flash from Windows: open the `.bin` in [QMK Toolbox](https://github.com/qmk/qmk_toolbox/releases). The file is reachable from Windows Explorer at `\\wsl$\<distro>\home\<user>\qmk-monsgeek\`. Flashing from inside WSL is possible with [usbipd-win](https://github.com/dorssel/usbipd-win), but it is more setup.
- **`/usr/bin/env: 'bash\r': No such file`:** the repo was checked out with Windows (CRLF) line endings. Clone it inside WSL, or run `git config core.autocrlf input` and clone again.

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
| `lib/chibios/...` or `lib/lufa/...: No such file` / `ChibiOS not found` | Submodules missing. Run the `git submodule update ...` line above. |
| `arm-none-eabi-gcc: not found` | No toolchain. Use the Docker way, or run `qmk doctor`. |
| `Could not find module appdirs!` / `Please run ... pip install -r /qmk_firmware/requirements.txt` | The latest QMK image is newer than the MonsGeek fork, which still needs older Python packages. Pull the latest `build.sh`, which installs the fork's requirements before compiling. With `--native`, run `python3 -m pip install -r ~/qmk-monsgeek/requirements.txt` once. |
| Docker: `permission denied` on files in `~/qmk-monsgeek` | Left over from an older build run. Run `sudo chown -R "$(id -u):$(id -g)" ~/qmk-monsgeek`. |
| Keyboard not detected for flashing | It is not in the bootloader, or it is in BT/2.4G mode. Use the cable and enter the bootloader again. |

---

## Credits

- Original keyboard support: [yangzheng20003](https://github.com/yangzheng20003) / MonsGeek.
- QMK: [qmk.fm](https://qmk.fm). Licensed GPL-2.0-or-later, like the original files.

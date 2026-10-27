# MonsGeek / AKKO M1 V5 (US) — özel QMK firmware'i

[English](readme.md) | **Türkçe**

M1 V5 US için değiştirilmiş firmware (VIA destekli; kablolu, Bluetooth ve 2.4G).

Standart MonsGeek firmware'ine göre farklar:

- **Daha hızlı çoklu tuş tepkisi:** QMK'nin varsayılan algoritması yerine tuş başına, beklemesiz debounce (`sym_eager_pk`). Varsayılan algoritma tüm klavyeye ortak çalışıyor ve basışı bekletiyordu. Artık her basış anında gönderiliyor ve başka tuşlar onu geciktirmiyor.
- **Daha parlak RGB:** azami parlaklık 104'ten 200'e çıkarıldı.
- **Daha akıcı RGB geçişleri:** RGB hız, parlaklık, renk ve mod değişikliklerinde yanıp sönme yok. RGB işleme hafifletildi, bu da tuş tepkisini hızlandırıyor.
- Varsayılan keymap'te **fare tuşları** açık.

Bu depo **yalnızca klavye klasörünü** içerir ve tek başına derlenmez. Kablosuz sürücü (`keyboards/linker/wireless`) yalnızca MonsGeek'in QMK fork'unda bulunduğu için o fork gerekir.

---

## Derleme

### Neler gerekiyor

| | Docker ile (önerilen) | Docker'sız |
|---|---|---|
| İşletim sistemi | Linux, macOS ya da Windows + WSL2 | Linux, macOS ya da Windows + MSYS2/WSL2 |
| Araçlar | `git`, `tar`, Docker | `git`, `tar`, [QMK CLI](https://docs.qmk.fm/newbs_getting_started) (`arm-none-eabi` derleyicisini de kurar) |
| Disk | ~1 GB (QMK imajı) + ~200 MB (kaynak) | ~200 MB (kaynak) + derleyici |
| İnternet | Yalnızca ilk derlemede | Yalnızca ilk derlemede |

Derleme iki kaynaktan oluşur:

- **QMK ağacı:** [`MonsGeek/qmk_firmware`](https://github.com/MonsGeek/qmk_firmware), **`wireless`** dalı. Resmi QMK ile **derlenmez**.
- **Bu depo:** o ağaçtaki `keyboards/monsgeek/m1_v5/m1_v5_us` klasörünün üzerine kopyalanır.

QMK'nin submodule'lerinden yalnızca 3'ü gerekli: `lib/chibios`, `lib/chibios-contrib` ve `lib/printf`. Yalnızca bunları indirmek ~20 saniye sürer. Tam bir `qmk setup` hepsini indirir (~1 GB).

### Tek komutla derleme (`build.sh`)

Depoda her şeyi tek seferde yapan bir [`build.sh`](build.sh) script'i var:

```bash
git clone https://github.com/atorata/m1_v5_us_firmware.git
cd m1_v5_us_firmware
./build.sh            # Docker ile (önerilen): bilgisayara derleyici kurmak gerekmez
./build.sh --native   # bilgisayarında kurulu QMK CLI ile
```

Script dört adım uygular:
1. MonsGeek QMK fork'unun `wireless` dalını `~/qmk-monsgeek` klasörüne sığ klonlar. Bu yalnızca ilk çalıştırmada yapılır.
2. Yukarıdaki 3 submodule'ü indirir.
3. Klavye klasörünü orijinal hâline döndürür, sonra bu deponun dosyalarını üzerine kopyalar.
4. Resmi `ghcr.io/qmk/qmk_cli` imajında ya da yerel `qmk` ile derler.

İlk çalıştırma bir iki dakika sürer: klon, submodule'ler ve imajın indirilmesi. Sonraki çalıştırmalar yalnızca kopyalayıp derler.

Çıktı: `~/qmk-monsgeek/monsgeek_m1_v5_m1_v5_us_default.bin`.

Varsayılanları iki ortam değişkeniyle değiştirebilirsin:
- `QMK_DIR`: QMK ağacının yeri.
- `KEYMAP`: derlenecek keymap.

Örnek: `KEYMAP=benim QMK_DIR=~/src/qmk ./build.sh`.

### Elle derleme (script'siz)

```bash
python3 -m pip install --user qmk           # ya da işletim sistemine göre QMK kurulum rehberi
git clone --depth 1 --branch wireless --single-branch \
    https://github.com/MonsGeek/qmk_firmware.git ~/qmk-monsgeek
cd ~/qmk-monsgeek
git submodule update --init --depth 1 lib/chibios lib/chibios-contrib lib/printf
tar -C /yol/m1_v5_us_firmware --exclude=.git --exclude=build.sh -cf - . | tar -C keyboards/monsgeek/m1_v5/m1_v5_us -xf -
qmk compile -kb monsgeek/m1_v5/m1_v5_us -km default
```

`qmk compile` derleyici bulamazsa `qmk doctor` çalıştır; `arm-none-eabi-gcc`'yi kurmayı önerir.

Kendi keymap'in için `keymaps/default` klasörünü `keymaps/<ad>` olarak kopyala ve `-km <ad>` ile derle.

### Windows (WSL2)

`build.sh` WSL2'de (örneğin Ubuntu) çalışır. Dikkat edilecekler:

- **Depoyu Linux dosya sisteminde klonla** (`~/...`), `/mnt/c/...` altında değil. `/mnt/c` altında derleme çok yavaşlar; dosya izinleri ve satır sonları da sorun çıkarır.
- **Docker:** iki yol var. Ya Docker Desktop'ı kurup *Settings → Resources → WSL integration* altında dağıtımını aç, ya da Docker Engine'i doğrudan WSL içine kur. `docker run --rm hello-world` ile kontrol et.
- **Docker'sız:** QMK CLI'ı WSL içine kurduysan `./build.sh --native` de çalışır. Derleyiciyi `qmk doctor` kurar.
- **Yükleme (flash):** WSL varsayılan olarak USB cihazlarını görmez. En kolayı Windows'tan yüklemek: `.bin` dosyasını [QMK Toolbox](https://github.com/qmk/qmk_toolbox/releases) ile aç. Dosyaya Windows Gezgini'nden `\\wsl$\<dağıtım>\home\<kullanıcı>\qmk-monsgeek\` yolundan ulaşabilirsin. [usbipd-win](https://github.com/dorssel/usbipd-win) ile WSL içinden yüklemek de mümkün, ama kurulumu daha uzun.
- **`/usr/bin/env: 'bash\r': No such file` hatası:** depo Windows satır sonlarıyla (CRLF) indirilmiş demektir. Depoyu WSL içinde klonla ya da `git config core.autocrlf input` yapıp yeniden klonla.

---

## Yükleme (flash)

İşlemci **WB32**, bootloader `wb32-dfu`. Yükleme **USB kablosuyla** yapılır; kablosuz modlarda yükleme yapılamaz.

1. **Bootloader'a gir** (biri yeterli):
   - Kablo takılıyken **Fn + Sağ Shift + Esc**. ⚠ Ayarları (VIA keymap, RGB) da sıfırlar.
   - Kabloyu takarken **Esc**'ye basılı tut (bootmagic). ⚠ Bu da ayarları sıfırlar.
   - PCB üzerindeki **reset düğmesine** bas.
2. **`.bin` dosyasını yükle** (biri yeterli):
   - [QMK Toolbox](https://github.com/qmk/qmk_toolbox/releases): `.bin` dosyasını aç, *Flash*'a bas.
   - Komut satırı, QMK ağacının içinden: `qmk flash -kb monsgeek/m1_v5/m1_v5_us -km default`. Bunun için `wb32-dfu-updater_cli` gerekir; `qmk doctor` ya da `qmk setup` kurar.
3. Kabloyu çıkarıp tak. Bootloader'a girmek ayarları sıfırladığı için VIA keymap'ini ve RGB ayarlarını yeniden yap.

---

## Sorun giderme

| Hata | Sebep / çözüm |
|---|---|
| `keyboards/linker/wireless/wireless.mk: No such file` | Resmi QMK kullanılıyor. `MonsGeek/qmk_firmware` deposunun `wireless` dalını kullan. |
| `undefined reference to 'im_test_rate_flag'` | `m1_v5_us.c` eski. Bu deponun son sürümünü çek; yeni MonsGeek dalları bu değişkeni istiyor. |
| `lib/chibios/... No such file` / `ChibiOS not found` | Submodule'ler eksik. Yukarıdaki `git submodule update ...` satırını çalıştır. |
| `arm-none-eabi-gcc: not found` | Derleyici yok. Docker ile derle ya da `qmk doctor` çalıştır. |
| `Could not find module appdirs!` / `Please run ... pip install -r /qmk_firmware/requirements.txt` | En güncel QMK imajı MonsGeek fork'undan daha yeni; fork hâlâ eski Python paketlerini istiyor. `build.sh`'ın son sürümünü çek: derlemeden önce fork'un bağımlılıklarını kendisi kuruyor. `--native` kullanıyorsan bir kez `python3 -m pip install -r ~/qmk-monsgeek/requirements.txt` çalıştır. |
| Docker: `~/qmk-monsgeek` içindeki dosyalarda `permission denied` | Eski bir derlemeden kalmış. `sudo chown -R "$(id -u):$(id -g)" ~/qmk-monsgeek` çalıştır. |
| Klavye yükleme için görünmüyor | Bootloader'da değil ya da BT/2.4G modunda. Kabloyu tak ve bootloader'a tekrar gir. |

---

## Teşekkür

- Orijinal klavye desteği: [yangzheng20003](https://github.com/yangzheng20003) / MonsGeek.
- QMK: [qmk.fm](https://qmk.fm). Orijinal dosyalar gibi GPL-2.0-or-later lisanslıdır.

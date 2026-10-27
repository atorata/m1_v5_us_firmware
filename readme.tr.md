# MonsGeek / AKKO M1 V5 **VIA** (US / ANSI) — özel QMK firmware'i

[English](readme.md) | **Türkçe**

**M1 V5'in VIA versiyonu, US (ANSI) düzeni** için değiştirilmiş firmware (kablolu, Bluetooth ve 2.4G).

> ⚠ **Önce klavyeni kontrol et.** Bu firmware yalnızca **US / ANSI** düzenli M1 V5 **VIA** versiyonu içindir:
> - USB kimlikleri **VID `0x342D`, PID `0xE4C5`**
> - VIA tasarım dosyası: [`M1_V5_VIA_V3_ANSI.json`](https://files.monsgeek.com/M1_V5_VIA_V3_ANSI.json)
>
> M1 V5'in diğer versiyonları farklı firmware kullanır: UK / ISO düzeni (`m1_v5_uk`, PID `0xE4C2`), VIA'sız sürümler ve diğer revizyonlar. Bu firmware'i onlara yükleme; orijinal firmware'ini geri yükleyene kadar klavye düzgün çalışmaz.
>
> Klavyenin kimliklerini görmek için kabloyla bağla:
> - **Windows:** Aygıt Yöneticisi → klavye → Özellikler → Ayrıntılar → *Donanım Kimlikleri*. İçinde `VID_342D&PID_E4C5` olmalı.
> - **Linux:** `lsusb | grep -i 342d` komutu `342d:e4c5` göstermeli.
> - **macOS:** Sistem Bilgisi → USB.

Standart MonsGeek firmware'ine göre farklar:

- **Daha hızlı çoklu tuş tepkisi:** QMK'nin varsayılan algoritması yerine tuş başına, beklemesiz debounce (`sym_eager_pk`). Varsayılan algoritma tüm klavyeye ortak çalışıyor ve basışı bekletiyordu. Artık her basış anında gönderiliyor ve başka tuşlar onu geciktirmiyor.
- **Daha parlak RGB:** azami parlaklık 104'ten 200'e çıkarıldı.
- **Daha akıcı RGB geçişleri:** RGB hız, parlaklık, renk ve mod değişikliklerinde yanıp sönme yok. RGB işleme hafifletildi, bu da tuş tepkisini hızlandırıyor.
- Varsayılan keymap'te **fare tuşları** açık.
- **Fn + G** sürekli yanan pil göstergelerini (şarj ışığı, düşük pil uyarısı) açar ya da kapatır. Ayar kaydedilir. Yeşil yanıp sönme açık, kırmızı kapalı demek. Fn + Space ile gösterilen pil seviyesi ve %0'da otomatik uyku bundan etkilenmez.
- **Fabrika ayarına dönüş NKRO'yu korur:** Fn + ` (3 sn basılı) sonrasında klavye, kablo çıkarılıp takılana kadar 6 tuşla sınırlı kalmıyor.
- **OpenRGB desteği** (yalnız kabloyla, VIA ile birlikte). Bkz. [OpenRGB](#openrgb).

Bu depo **yalnızca klavye klasörünü** içerir ve tek başına derlenmez. Kablosuz sürücü (`keyboards/linker/wireless`) yalnızca MonsGeek'in QMK fork'unda bulunduğu için o fork gerekir.

---

## İndirme (derlemeden)

Yüklemeye hazır firmware [**Releases**](https://github.com/atorata/m1_v5_us_firmware/releases) sayfasında.

1. En son sürümü aç ve `m1_v5_via_us_ansi_<sürüm>.bin` dosyasını indir.
2. İstersen dosyayı yanındaki `.sha256` ile doğrula: `sha256sum -c m1_v5_via_us_ansi_<sürüm>.bin.sha256`.
3. [Yükleme (flash)](#yükleme-flash) bölümündeki gibi klavyeye yükle.

**Pre-release** olarak işaretlenen sürümler (`v1.2-alpha` gibi etiketler) henüz bir klavyede denenmedi; yalnızca denemeye yardım etmek istiyorsan kullan.

Her sürümü GitHub Actions bu depodan, aşağıda anlatılan `build.sh` ile otomatik derler. Henüz sürüm olmamış commit'lerin derlemeleri, [Actions](https://github.com/atorata/m1_v5_us_firmware/actions) sayfasındaki her çalışmaya `firmware` adıyla eklenir; bunları indirmek için GitHub'a giriş yapmış olman gerekir.

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

QMK'nin submodule'lerinden yalnızca 4'ü gerekli: `lib/chibios`, `lib/chibios-contrib`, `lib/printf` ve `lib/lufa` (ChibiOS'un USB katmanı LUFA'nın başlık dosyalarını kullanıyor). Yalnızca bunları indirmek ~20 saniye sürer. Tam bir `qmk setup` hepsini indirir (~1 GB).

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
2. Yukarıdaki 4 submodule'ü indirir.
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
git submodule update --init --depth 1 lib/chibios lib/chibios-contrib lib/printf lib/lufa
tar -C /yol/m1_v5_us_firmware --exclude=.git --exclude=.github --exclude=build.sh -cf - . | tar -C keyboards/monsgeek/m1_v5/m1_v5_us -xf -
qmk compile -kb monsgeek/m1_v5/m1_v5_us -km default
```

`qmk compile` derleyici bulamazsa `qmk doctor` çalıştır; `arm-none-eabi-gcc`'yi kurmayı önerir.

Kendi keymap'in için `keymaps/default` klasörünü `keymaps/<ad>` olarak kopyala ve `-km <ad>` ile derle.

### Tarama hızını ölçme (`scanrate` keymap)

Ölçüm amaçlı derleme: varsayılan keymap'e ek olarak **Fn + S**, klavyenin son
bir saniyede matrisi kaç kez taradığını yazar (ör. `scan 1834`). Bir metin
düzenleyici açıp Fn + S'ye birkaç kez bas; RGB efektleri, RGB kapalı ve
kablolu/kablosuz arasında karşılaştır.

```sh
KEYMAP=scanrate ./build.sh
```

Her CI koşusu bunu `firmware-scanrate` artifact'i olarak da yükler. Release'lere
eklenmez.

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
   - Kablo takılıyken **Fn + Sağ Shift + Esc**. v1.3'ten beri ayarları korur (3. adıma bak).
   - Kabloyu takarken **Esc**'ye basılı tut (bootmagic). ⚠ Bu tüm ayarları sıfırlar.
   - PCB üzerindeki **reset düğmesine** bas.
2. **`.bin` dosyasını yükle** (biri yeterli):
   - [QMK Toolbox](https://github.com/qmk/qmk_toolbox/releases): `.bin` dosyasını aç, *Flash*'a bas.
   - Komut satırı, QMK ağacının içinden: `qmk flash -kb monsgeek/m1_v5/m1_v5_us -km default`. Bunun için `wb32-dfu-updater_cli` gerekir; `qmk doctor` ya da `qmk setup` kurar.
3. Kabloyu çıkarıp tak.
   - **Korunanlar** (Fn + Sağ Shift + Esc ile girildiyse, v1.3 ve sonrası): RGB modu, rengi ve parlaklığı; Fn + G, Fn + W ve Fn + Sağ Ctrl seçimleri; NKRO.
   - **VIA'nın sıfırladıkları:** Yeni firmware farklı bir günde derlenmişse, ki her sürümde öyle, **VIA keymap'in ve makroların**. Yüklemeden önce not al ya da VIA'dan dışa aktar (*Save + Load*).
4. **VIA:** [VIA](https://usevia.app) klavyeyi kendiliğinden tanımazsa MonsGeek'in tasarım dosyasını yükle: [`M1_V5_VIA_V3_ANSI.json`](https://files.monsgeek.com/M1_V5_VIA_V3_ANSI.json). Bunun için önce *Settings → Show Design tab*, sonra *Design → Load*.

### Linux (komut satırı)

QMK Toolbox yalnızca Windows ve macOS'ta çalışır. Linux'ta yükleme, WB32 bootloader'ının kendi aracı olan `wb32-dfu-updater_cli` ile yapılır; `qmk flash` da arka planda bu aracı kullanır.

**1. `wb32-dfu-updater_cli`'ı kur (bir kez).** Derleme araçlarını ve libusb'yi kur, sonra aracı derle:

```bash
# Debian / Ubuntu / Mint
sudo apt install git cmake build-essential libusb-1.0-0-dev
# Fedora:  sudo dnf install git cmake gcc-c++ libusb1-devel
# Arch:    sudo pacman -S --needed git cmake base-devel libusb

git clone --depth 1 https://github.com/WestberryTech/wb32-dfu-updater.git
cd wb32-dfu-updater
sudo bash ./bootstrap.sh install      # derleyip /usr/local/bin'e kurar
wb32-dfu-updater_cli --help           # kurulduğunu kontrol et
```

Hazır paketler [wb32-dfu-updater README](https://github.com/WestberryTech/wb32-dfu-updater#pre-built-packages)'sinde listeli. Fedora için bir COPR deposu var, diğer dağıtımlar Homebrew kullanabilir.

**2. İsteğe bağlı: `sudo`'suz yükleme.** Normal kullanıcının bootloader'a (USB kimliği `342d:dfa0`) erişmesine izin ver:

```bash
echo 'SUBSYSTEMS=="usb", ATTRS{idVendor}=="342d", ATTRS{idProduct}=="dfa0", TAG+="uaccess"' \
    | sudo tee /etc/udev/rules.d/50-wb32-dfu.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

**3. Yükle.** Kabloyu tak ve bootloader'a gir (**Fn + Sağ Shift + Esc**, yukarıya bak). Sonra:

```bash
lsusb | grep -i 342d:dfa0                                   # bootloader görünmeli
wb32-dfu-updater_cli -l                                     # "Found DFU: [0x342D:0xDFA0] ..." yazmalı
wb32-dfu-updater_cli -D m1_v5_via_us_ansi_<sürüm>.bin       # firmware'i yaz
wb32-dfu-updater_cli -R                                     # klavyeyi yeniden başlat
```

2. adımı atladıysan bu komutların başına `sudo` ekle. Firmware'i kendin derlediysen dosya `~/qmk-monsgeek/monsgeek_m1_v5_m1_v5_us_default.bin`.

`lsusb` `342d:dfa0` yerine `342d:e4c5` gösteriyorsa klavye normal çalışıyordur, bootloader'da değildir; bootloader'a tekrar gir.

**WSL2:** WSL varsayılan olarak USB cihazlarını görmez. Windows'tan QMK Toolbox ile yükle ya da klavye bootloader'dayken [usbipd-win](https://github.com/dorssel/usbipd-win) ile WSL'e bağla.

---

## OpenRGB

Firmware, [OpenRGB](https://openrgb.org) 1.0 ve sonrasının desteklediği VialRGB protokolünü
konuşuyor. VIA ile aynı USB kanalını kullanıyor; VIA çalışmaya devam ediyor, geçiş yapmak gerekmiyor.

1. Klavyeyi **kabloyla** bağla (kablosuz modlar bu kanalı taşımıyor).
2. OpenRGB'de: **Settings → Manual Devices → Add → QMK VialRGB Device**.
   Name: `M1 V5`, USB VID: `342D`, USB PID: `E4C5`. Kaydet, cihazları yeniden tara (ya da OpenRGB'yi yeniden başlat).
3. Klavye tüm efektleriyle görünür; tuş tuş renk için **Direct** modu da var.

- OpenRGB'den yapılan değişiklikler klavyeye **kaydedilmez**. Kabloyu çıkarıp takmak ya da
  klavyeden efekt seçmek (Fn + Delete) klavyenin kendi ışığını geri getirir.
- OpenRGB'nin tanımadığı efektler (pixel flow, RGB kaydı) OpenRGB'de *Off* görünür.
- VIA ile OpenRGB'yi aynı anda açık tutma: ikisi aynı kanalı okur ve birbirinin cevabını alabilir.
- Linux'ta klavyenin hidraw cihazına erişim gerekir (tarayıcıdaki VIA da aynısını ister).
  OpenRGB klavyeyi bulamazsa bu kuralı ekleyip kabloyu çıkar-tak:

   ```sh
   echo 'KERNEL=="hidraw*", ATTRS{idVendor}=="342d", ATTRS{idProduct}=="e4c5", TAG+="uaccess"' \
       | sudo tee /etc/udev/rules.d/60-m1v5-hidraw.rules
   sudo udevadm control --reload-rules && sudo udevadm trigger
   ```

## Sorun giderme

| Hata | Sebep / çözüm |
|---|---|
| `keyboards/linker/wireless/wireless.mk: No such file` | Resmi QMK kullanılıyor. `MonsGeek/qmk_firmware` deposunun `wireless` dalını kullan. |
| `undefined reference to 'im_test_rate_flag'` | `m1_v5_us.c` eski. Bu deponun son sürümünü çek; yeni MonsGeek dalları bu değişkeni istiyor. |
| `lib/chibios/...` ya da `lib/lufa/...: No such file` / `ChibiOS not found` | Submodule'ler eksik. Yukarıdaki `git submodule update ...` satırını çalıştır. |
| `arm-none-eabi-gcc: not found` | Derleyici yok. Docker ile derle ya da `qmk doctor` çalıştır. |
| `Could not find module appdirs!` / `Please run ... pip install -r /qmk_firmware/requirements.txt` | En güncel QMK imajı MonsGeek fork'undan daha yeni; fork hâlâ eski Python paketlerini istiyor. `build.sh`'ın son sürümünü çek: derlemeden önce fork'un bağımlılıklarını kendisi kuruyor. `--native` kullanıyorsan bir kez `python3 -m pip install -r ~/qmk-monsgeek/requirements.txt` çalıştır. |
| Docker: `~/qmk-monsgeek` içindeki dosyalarda `permission denied` | Eski bir derlemeden kalmış. `sudo chown -R "$(id -u):$(id -g)" ~/qmk-monsgeek` çalıştır. |
| Klavye yükleme için görünmüyor | Bootloader'da değil ya da BT/2.4G modunda. Kabloyu tak ve bootloader'a tekrar gir. |

---

## Teşekkür

- Orijinal klavye desteği: [yangzheng20003](https://github.com/yangzheng20003) / MonsGeek.
- QMK: [qmk.fm](https://qmk.fm). Orijinal dosyalar gibi GPL-2.0-or-later lisanslıdır.
- Kablosuz sürücü bağlantısı (`wls/`): [`MonsGeek/qmk_firmware`](https://github.com/MonsGeek/qmk_firmware) deposunun `wireless` dalından değiştirilmeden alındı.

## Lisans

GPL-2.0-or-later. Tam metin için [`LICENSE`](LICENSE) dosyasına bakın. Her kaynak dosya kendi orijinal telif bildirimini korur. Yayımlanan her `.bin` dosyasının kaynağı, bu depodaki ilgili etiketli commit'tir ve `MonsGeek/qmk_firmware` (`wireless` dalı) ile derlenir.

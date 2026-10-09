# BlueRetro-WiiU-USB (eksperymentalny prototyp)

Dwie plytki: **LOLIN32 Lite** (pad DualSense / Bluepad32 / BT Classic) oraz **ESP32-S2 Mini** (UART do testowego USB HID).

> **Uwaga:** kod S2 emuluje **GENERYCZNY** kontroler USB HID. To nie jest emulacja Sony DualSense i **nie potwierdza zgodnosci z Nintendont / Wii VC Inject**. Projekt to baza do testowania, a nie gotowy adapter Wii U. Nie zmieniaj konfiguracji Wii U na potrzeby pierwszego testu.

## Polaczenie

| LOLIN32 Lite | ESP32-S2 Mini |
|---|---|
| GPIO17 TX | GPIO16 RX |
| GPIO16 RX (opcjonalnie) | GPIO17 TX (opcjonalnie) |
| GND | GND |

Obie plytki pracuja z logika 3.3 V. Dla jednokierunkowej transmisji wystarczy TX -> RX i GND. Na poczatku zasilaj je osobno przez USB; **nie lacz linii 5 V/VBUS ani 3V3**. USB S2 podpina sie do komputera przez port danych. Sprawdz oznaczenia pinow na posiadanych plytkach. GPIO19/20 S2 sa zarezerwowane przez natywne USB.

## 1. USB: PlatformIO i komputer

Otworz folder `usb-s2` w VS Code PlatformIO i uzyj **Build**, nastepnie **Upload** (`pio run -t upload`).
ESP32-S2 czasem wymaga wejscia w download mode: przytrzymaj BOOT, nacisnij RESET, zwolnij BOOT, potem wgraj firmware. Po resecie system komputera powinien widziec USB HID gamepad **ESP32-S2 UART Gamepad Test**. Test bez UART pokaze neutralne osie i przyciski; to oczekiwane.

Na Windows sprawdz w `joy.cpl` (Win+R). Na Linux mozna uzyc `evtest` lub `jstest`. Nie ustawiaj numerow Sony VID/PID: samo skopiowanie identyfikatorow nie zapewni zgodnosci USB.

## 2. Bluetooth: oficjalny upstream Bluepad32

Bluepad32 **nie dziala** po zwyklym wpisaniu `lib_deps = bluepad32` w typowym `framework=arduino`. Jego Arduino API wymaga specjalnego core lub oficjalnego szablonu ESP-IDF z Bluepad32.

1. Sklonuj **osobno** `https://github.com/ricardoquesada/esp-idf-arduino-bluepad32-template` poleceniem `git clone --recursive`.
2. Otworz sklonowany katalog w VS Code PlatformIO i skompiluj przyklad **bez zmian** (upewnij sie, ze toolchain dziala).
3. W upstream `platformio.ini` dodaj srodowisko `board = lolin32_lite` przez skopiowanie dostarczonego srodowiska ESP32 (`esp32dev`). Zachowaj pozostale ustawienia, framework i przypiete wersje upstream.
4. Plik `bluetooth-esp32/bluepad32_sender.cpp` jest **fragmentem integracyjnym** dla upstream; przenies jego `setup()` / `loop()` do istniejącego punktu wejscia Arduino w szablonie, zamiast dodawac drugie `app_main()`. Skopiuj `shared/pad_packet.h` do sciezki include dostepnej dla kompilatora.
5. Najpierw sprawdz parowanie: trzymaj **Create + PS** na DualSense, az zacznie pulsowac LED. Po polaczeniu ESP32 wysyla ramki UART 100 Hz.

**Status:** ten folder nie zawiera kopii kodu Bluepad32 ani jego submodulow. Ze wzgledu na zaleznosci i roznice wersji plik nadajnika moze wymagac drobnych zmian nazw stalych API po skopiowaniu do danej wersji szablonu. Nie jest to samodzielne srodowisko Build.

## 3. UART i bezpieczenstwo

Strumien: ramki 20 bajtow zaczynajace sie `A5 5A`, kontrola CRC-8, 115200 8N1, 100Hz. Szczegoly w `shared/pad_packet.h`.

Po utracie ramek na ponad 120 ms S2 zeruje osie i przyciski. W przypadku braku Bluetooth nadajnik wysyla `connected=0`.

## 4. Wii U / Nintendont (NIEZWERYFIKOWANE)

Gry GameCube uruchamiane jako Wii VC Inject moga uzywac Nintendont, jednak obsluga USB HID zalezy od wersji Nintendont, konfiguracji i typu injecta. Aktualny firmware uzywa **generic USB gamepad** — Nintendont nie ma obowiazku go rozpoznac. Kolejny etap: wybrac **zweryfikowany** profil HID, descriptor/report oraz pasujacy `controller.ini` lub plik kontrolera, i przetestowac go na konkretnej instalacji. Emulacja prawdziwego DualSense wymagalaby pelnych deskryptorow i raportow USB, nie samego VID/PID.

**Najpierw test USB na PC, potem test UART, na koniec Wii U.**

## Test protokolu

`python3 tools/test_packets.py`

## Linki / zrodla

- Bluepad32 / oficjalny szablon: https://github.com/ricardoquesada/esp-idf-arduino-bluepad32-template
- Bluepad32: https://bluepad32.readthedocs.io/en/latest/plat_arduino/
- PlatformIO LOLIN32 Lite: https://docs.platformio.org/en/stable/boards/espressif32/lolin32_lite.html
- PlatformIO LOLIN S2 Mini: https://docs.platformio.org/en/stable/boards/espressif32/lolin_s2_mini.html
- Nintendont DualSense mapping: https://github.com/FIX94/Nintendont/blob/master/controllerconfigs/controller_ps5.ini

## GitHub Actions (CI)

Workflow: `.github/workflows/build.yml`.

- Uruchamia sie po `push` na `main` / `master`, przy pull requestach oraz recznie w zakladce **Actions** -> **Build ESP32-S2 firmware** -> **Run workflow**.
- Weryfikuje protokol UART (`python tools/test_packets.py`).
- Kompiluje firmware ESP32-S2 Mini w PlatformIO (`python -m platformio run -d usb-s2 -e lolin_s2_mini`).
- Po udanym buildzie udostepnia pliki `firmware.bin`, `bootloader.bin` i `partitions.bin` jako artefakt `lolin-s2-mini-firmware`.

**Ograniczenia:** folder `bluetooth-esp32` zawiera tylko fragment integracyjny Bluepad32, a nie kompletne samodzielne srodowisko PlatformIO. CI **nie kompiluje jeszcze firmware LOLIN32 Lite**. Workflow wymaga pobrania narzedzi i pakietow z internetu podczas uruchomienia w GitHub Actions. Samo przejscie kompilacji nie dowodzi zgodnosci USB HID z Nintendont.

## Stage 2: LOLIN32 Lite / DualSense

The `bluetooth-esp32` folder contains the Bluepad32 sketch and integration notes. `.github/workflows/build.yml` includes an experimental independent LOLIN32 Lite job based on the official Bluepad32 ESP-IDF Arduino template. Its build has not yet been verified in GitHub Actions. Review the job log before flashing. The working ESP32-S2 firmware remains unchanged.

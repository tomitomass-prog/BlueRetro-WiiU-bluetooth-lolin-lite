# LOLIN32 Lite / Bluepad32

Bluepad32 needs its own ESP-IDF / Arduino integration. This folder contains the `sketch.cpp` replacement to be copied into the **official** [esp-idf-arduino-bluepad32-template](https://github.com/ricardoquesada/esp-idf-arduino-bluepad32-template). A normal `lib_deps = Bluepad32` in stock Arduino PlatformIO is not sufficient.

The GitHub Actions job `Build LOLIN32 Lite (Bluepad32)` clones the upstream template with submodules, copies `bluepad32_sender.cpp` to `main/sketch.cpp`, copies `shared/pad_packet.h` to `main/pad_packet.h`, creates a PlatformIO environment with `board = lolin32_lite` and compiles it. The build has not yet been validated on GitHub or hardware.

Connections: LOLIN32 GPIO17 (TX) -> ESP32-S2 GPIO16 (RX); GND -> GND. The return line LOLIN32 GPIO16 RX <- S2 GPIO17 TX is optional, because this first version is transmit-only. Use **3.3V** logic. During standalone tests, power each board through its own USB and do not link 5V/3V3 rails. Test with the S2 connected to the Windows PC first, not Wii U.

Pair DualSense by holding **Create + PS** until the light flashes; power the LOLIN32 Lite after flashing. This Bluetooth adapter has a different Bluetooth address than the existing BlueRetro; pair with this new adapter and keep the old one untouched.

It transmits 20-byte packets at 115200 baud, ~100 Hz, compatible with `shared/pad_packet.h` and existing S2 firmware. The HOME button mapping and Nintendo-specific layout are pending. A successful GitHub build does not guarantee Bluetooth pairing or Nintendont support.

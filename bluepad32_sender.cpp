// LOLIN32 Lite / Bluepad32 Arduino integration (official ESP-IDF template).
// Place this file as main/sketch.cpp in the official Bluepad32 template.
// UART: GPIO17 TX -> S2 GPIO16 RX; GND -> GND. Optional return GPIO16 RX <- S2 GPIO17 TX.
#include <Arduino.h>
#include <Bluepad32.h>
#include "pad_packet.h"

static ControllerPtr controllers[BP32_MAX_GAMEPADS] = {};
static uint8_t sequence = 0;
static uint32_t lastTx = 0;

void onConnectedController(ControllerPtr ctl) {
  for (auto &slot : controllers) {
    if (!slot) { slot = ctl; Console.println("Gamepad connected"); return; }
  }
  Console.println("No free controller slot");
}
void onDisconnectedController(ControllerPtr ctl) {
  for (auto &slot : controllers) if (slot == ctl) slot = nullptr;
  Console.println("Gamepad disconnected");
}
static int8_t clampAxis(int v) {
  if (v < -512) v = -512;
  if (v > 511) v = 511;
  return (int8_t)(v / 4);
}
static uint8_t clampTrigger(int v) {
  if (v < 0) v = 0;
  if (v > 1023) v = 1023;
  return (uint8_t)(v / 4);
}
static uint8_t encodeHat(uint8_t d) {
  const bool u = d & DPAD_UP, down = d & DPAD_DOWN;
  const bool r = d & DPAD_RIGHT, l = d & DPAD_LEFT;
  if (u && r) return 2;
  if (down && r) return 4;
  if (down && l) return 6;
  if (u && l) return 8;
  if (u) return 1;
  if (r) return 3;
  if (down) return 5;
  if (l) return 7;
  return 0;
}
void setup() {
  // Console is Bluepad32 debug output; Serial1 is separate binary UART.
  Serial1.begin(115200, SERIAL_8N1, 16, 17);
  BP32.setup(&onConnectedController, &onDisconnectedController, true);
  BP32.enableVirtualDevice(false);
  Console.println("LOLIN32 Lite: DualSense -> UART ready");
  // Do not call forgetBluetoothKeys() at every boot: preserves pairing.
}
void loop() {
  BP32.update();
  if ((uint32_t)(millis() - lastTx) < 10) { delay(1); return; }
  lastTx = millis();
  ControllerPtr pad = nullptr;
  for (auto ctl : controllers) {
    if (ctl && ctl->isConnected() && ctl->isGamepad()) { pad = ctl; break; }
  }
  uint8_t out[PAD_PACKET_SIZE];
  if (!pad) {
    pad_encode(out, sequence++, 0, 0, 0, 0, 0, 0, 0, 0, false);
  } else {
    uint16_t b = 0;
    const uint16_t bs = pad->buttons();
    if (bs & BUTTON_A) b |= PAD_A;
    if (bs & BUTTON_B) b |= PAD_B;
    if (bs & BUTTON_X) b |= PAD_X;
    if (bs & BUTTON_Y) b |= PAD_Y;
    if (bs & BUTTON_SHOULDER_L) b |= PAD_L;
    if (bs & BUTTON_SHOULDER_R) b |= PAD_R;
    if (bs & BUTTON_TRIGGER_L) b |= PAD_ZL;
    if (bs & BUTTON_TRIGGER_R) b |= PAD_ZR;
    if (bs & BUTTON_START) b |= PAD_START;
    if (bs & BUTTON_SELECT) b |= PAD_SELECT;
    if (bs & BUTTON_THUMB_L) b |= PAD_L3;
    if (bs & BUTTON_THUMB_R) b |= PAD_R3;
    // HOME will be mapped separately after verifying miscButtons() masks.
    pad_encode(out, sequence++, b, encodeHat(pad->dpad()),
      clampAxis(pad->axisX()), clampAxis(pad->axisY()),
      clampAxis(pad->axisRX()), clampAxis(pad->axisRY()),
      clampTrigger(pad->brake()), clampTrigger(pad->throttle()), true);
  }
  Serial1.write(out, sizeof(out));
  delay(1);
}

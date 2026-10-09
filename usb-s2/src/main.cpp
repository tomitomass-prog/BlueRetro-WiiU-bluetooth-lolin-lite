#include <Arduino.h>
#include <USB.h>
#include <USBHIDGamepad.h>
#include "pad_packet.h"

// USB port: native USB on GPIO19/20, managed by USB stack.
// UART: RX=GPIO16 from LOLIN32 GPIO17 TX; optional reverse TX=GPIO17.
static constexpr int UART_RX = 16, UART_TX = 17;
static constexpr uint32_t BAUD = 115200;
static constexpr unsigned TIMEOUT_MS = 120;
static USBHIDGamepad gamepad;
static uint8_t frame[PAD_PACKET_SIZE];
static uint8_t pos=0;
static uint32_t lastGood=0, lastReport=0;
static bool live=false;
static uint8_t lx=0, ly=0, rx=0, ry=0, lt=0, rt=0, hat=0;
static uint16_t buttons=0;

static int8_t triggerToAxis(uint8_t t) {return (int8_t)((int)t-128);}
static void neutral() {lx=ly=rx=ry=lt=rt=0; hat=0; buttons=0;}
static void accept(const uint8_t* f) {
  if(!(f[12]&PAD_CONNECTED)) {neutral();live=false; return;}
  buttons=(uint16_t)f[3]|((uint16_t)f[4]<<8);
  hat=f[5]; // Protocol 0 center, 1 up, 2 up-right, etc; matches USBHIDGamepad.
  lx=f[6];ly=f[7];rx=f[8];ry=f[9];lt=f[10];rt=f[11];
  live=true;lastGood=millis();
}
static void receive() {
 while(Serial1.available()) {
  uint8_t b=(uint8_t)Serial1.read();
  if(pos==0 && b!=PAD_SYNC0)continue;
  if(pos==1 && b!=PAD_SYNC1){pos=(b==PAD_SYNC0)?1:0;continue;}
  frame[pos++]=b;
  if(pos==PAD_PACKET_SIZE) { if(pad_valid(frame))accept(frame);pos=0; }
 }
}
void setup() {
 Serial1.begin(BAUD,SERIAL_8N1,UART_RX,UART_TX);
 neutral();
 // Generic USB gamepad; intentionally NOT claiming to be a Sony DualSense.
 USB.productName("ESP32-S2 UART Gamepad Test");
 USB.manufacturerName("DIY Prototype");
 gamepad.begin();
 USB.begin();
}
void loop() {
 receive();
 if(live && millis()-lastGood>TIMEOUT_MS) {neutral();live=false;}
 if(millis()-lastReport>=10) {
   lastReport=millis();
   // signed axes sourced from int8 payload; USBHIDGamepad send takes signed axes
   gamepad.send((int8_t)lx,(int8_t)ly,(int8_t)rx,(int8_t)ry,
                triggerToAxis(lt),triggerToAxis(rt),hat,(uint32_t)buttons);
 }
 delay(1);
}

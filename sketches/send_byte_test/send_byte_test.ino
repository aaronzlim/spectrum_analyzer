#include "my_neopixel.h"

void setup() {
  ledSetup();
}

#define DELAY 1000 // 1 second
#define DELAY_U 6  // 6 microseconds

void loop() {
  cli();
  sendByte(0);     // G = 0
  sendByte(0x30);  // R = 128
  sendByte(0);     // B = 0
  _delay_us(DELAY_U);
  sei();
  delay(DELAY);
  cli();
  sendByte(0);
  sendByte(0);
  sendByte(0);
  _delay_us(DELAY_U);
  sei();
  delay(DELAY);
}

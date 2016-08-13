#include "my_neopixel.h"

void setup() {
  DDRD |= B10000000;  // Make pin 6 an output
  PORTD |= B00100000; // set all digital pins low
}

unsigned long int t = 0;
void loop() {
  for(t = 0; t < 250000; t++) {
    mySendBit();
  }
  delay(2000);
}

void mySendBit() {
  asm volatile (
          "sbi %[port], %[BIT] \n\t"        // Set the output bit
          ".rept %[onCycles] \n\t"          // Execute NOPs to delay exactly the specified number of cycles
          "nop \n\t"
          ".endr \n\t"
          "cbi %[port], %[BIT] \n\t"        // Clear the output bit
          ".rept %[offCycles] \n\t"         // Execute NOPs to delay exactly the specified number of cycles
          "nop \n\t"
          ".endr \n\t"
          ::
          [port]      "I" (_SFR_IO_ADDR(PIXEL_PORT)),
          [BIT]       "I" (TOGGLE_BIT),
          [onCycles]  "I" (63),  // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer
          [offCycles] "I" (63)   // Minimum interbit delay
        );
}

/*
Author: Aaron Lim
Contact: azl4.dev@gmail.com

Description: This is the main program to control neopixel
             spectrum analyzer.
*/

#include "my_neopixel.h"
/*
 * The my_neopixel.h file includes:
 *     PIXEL_PORT  PORTD
 *     PIXEL_DDR   DDRD
 *     PIXEL_PIN   6
 *     NUM_COLS    21
 *     NUM_ROWS    10
 *     COL_WIDTH   3
 *     NUM_BANDS   ( NUM_COLS / COL_WIDTH )
 *     NUM_PIXELS  ( NUM_COLS * NUM_ROWS )
 */
 
void setup() {
    // Initialize Neopixels
    ledSetup();
    setBrightness(50);
    Clear();
    TIMSK0 = 0;    // Turn off timer0 for lower jitter
}


uint8_t i = 0; // Initialize iterators
uint8_t j = 0;

uint8_t newBands[NUM_BANDS] = {};    // Array to hold new band amplitudes
uint8_t GRB_vals[NUM_PIXELS*3] = {}; // Array to hold pixel GRB values

unsigned long interval = 2000;  // Waiting interval

void loop() {
  
  uint8_t newBands[NUM_BANDS] = {};
  for(i = 0; i < NUM_BANDS; i++) {
    newBands[i] = i+2;
  }

  setMatrix(newBands, GRB_vals);
  show(GRB_vals);
  _delay_ms(1000);
  Clear();
  _delay_ms(500);
}

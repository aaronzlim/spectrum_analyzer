#include "my_neopixel.h"

#define DELAY_MS 25
void setup() {
  ledSetup();
  setBrightness(30);
  //setColorScheme(Rainbow); // Currently have GYR and Rainbow
  Clear();
  TIMSK0 = 0; // Reduces jitter
}

uint8_t i = 0;
uint8_t j = 0;
uint8_t newBands[NUM_BANDS] = {};
uint8_t GRB_vals[NUM_PIXELS*3] = {};

void loop() {
  for(i = 0; i < NUM_BANDS; i++) {
    // Make bars go up
    for(j = 0; j < NUM_ROWS; j++) {

      newBands[i] = j + 1;
      setMatrix(newBands, GRB_vals);
      show(GRB_vals);
      _delay_ms(DELAY_MS);

    }
    // Make bars come down
    for(j = 0; j < NUM_ROWS; j++) {

      newBands[i] = NUM_ROWS - j - 1;
      setMatrix(newBands, GRB_vals);
      show(GRB_vals);
      _delay_ms(DELAY_MS);

    } 
  }
  for(i = (NUM_BANDS); i > 0; i--) {
    // Make bars go up
    for(j = 0; j < NUM_ROWS; j++) {

      newBands[i-1] = j + 1;
      setMatrix(newBands, GRB_vals);
      show(GRB_vals);
      _delay_ms(DELAY_MS);
    }
    
    // Make bars come down
    for(j = 0; j < NUM_ROWS; j++) {

      newBands[i-1] = NUM_ROWS - j - 1;
      setMatrix(newBands, GRB_vals);
      show(GRB_vals);
      _delay_ms(DELAY_MS);
    } 
  }
}


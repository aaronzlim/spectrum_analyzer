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
    pinMode(PIXEL_PIN, OUTPUT);
    
    // Initialize Neopixels
    ledSetup();
    setBrightness(128);
    Clear();

    TIMSK0 = 0;    // Turn off timer0 for lower jitter

    // Eventually need to set up an analog pin to receive audio
    // and maybe a pin to adjust gain
}


void loop() {
    uint8_t pixels[3*NUM_PIXELS]; // Each neopixel needs 3 bytes of data for GRB values
    uint8_t newBars[NUM_BANDS] = { }; // Array to hold current value for each band

    // This is just to test functionality...
    for(uint8_t i = 0; i < NUM_BANDS; i++) {
        for(uint8_t k = 0; k < NUM_BANDS; k++) {newBars[k] = 0;} // Clear array
        for(uint8_t j = 0; j < NUM_ROWS; j++) {
            newBars[i] = j;
            setMatrix(newBars, pixels);
            show(pixels);
            delay(500);
        }
    }
}

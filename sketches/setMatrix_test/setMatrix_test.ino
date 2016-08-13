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
    setBrightness(128);
    Clear();

    TIMSK0 = 0;    // Turn off timer0 for lower jitter

    Serial.begin(9600); // USED FOR DEBUGGING
    while(!Serial);     // Need to get rid of this when complete

    // Eventually need to set up an analog pin to receive audio
    // And maybe a pin to adjust gain
    // Also maybe a pin to select which color scheme to use (see my_neopixel.h)
    
// TESTING --------------------------------------------------------------------------

    uint8_t pixels[3*NUM_PIXELS]; // Each neopixel needs 3 bytes of data for GRB values
    uint8_t newBars[NUM_BANDS] = { }; // Array to hold current value for each band

    // This is just to test functionality...
    for(unsigned int i = 0; i < NUM_BANDS; i++) {
        newBars[i] = i+3;
    }
    unsigned long start = micros();
    setMatrix(newBars, pixels);
    unsigned long sto = micros();

    Serial.print("setMatrix() took "); Serial.print(sto-start); Serial.print(" us");
    

    unsigned int k = 0;
    for(uint8_t i = 0; i < NUM_BANDS; i++) {
        Serial.print("\n------------------ BAND ");
        Serial.print(i);
        Serial.print(" ------------------\n");
        for ( unsigned int j = 0; j < NUM_ROWS*3; j+=3 ) {
            if( (i % 2 ) == 0 ) {  // Even numbered band

                k = i * 90;

                Serial.print("(");
                Serial.print(pixels[k+j]); 
                Serial.print(",");
                Serial.print(pixels[k+1+j]); 
                Serial.print(",");
                Serial.print(pixels[k+2+j]);
                Serial.print(")\t(");
                Serial.print(pixels[k+57-j]); 
                Serial.print(",");
                Serial.print(pixels[k+58-j]);
                Serial.print(",");
                Serial.print(pixels[k+59-j]);
                Serial.print(")\t("); 
                Serial.print(pixels[k+60+j]);
                Serial.print(",");
                Serial.print(pixels[k+61+j]);
                Serial.print(",");Serial.print(pixels[k+62+j]);
                Serial.print(")\n");
        
            } else {  // Odd numbered band
        
                k = (i * 90) + 27;
                Serial.print("(");
                Serial.print(pixels[k-j]);
                Serial.print(",");
                Serial.print(pixels[k+1-j]);
                Serial.print(",");
                Serial.print(pixels[k+2-j]);
                Serial.print(")\t(");
                Serial.print(pixels[k+3+j]);
                Serial.print(",");
                Serial.print(pixels[k+4+j]);
                Serial.print(",");
                Serial.print(pixels[k+5+j]);
                Serial.print(")\t(");
                Serial.print(pixels[k+60-j]);
                Serial.print(",");
                Serial.print(pixels[k+61-j]);
                Serial.print(",");
                Serial.print(pixels[k+62-j]);
                Serial.print(")\n");
        
            }
        }
    }
}

void loop() {
  
}


/*
 * Hello and welcome to my arduino+processing spectrum analyzer project!!
 * Below is the code that runs the neopixels ONLY
 * All sound processing is done in processing and is shipped to our arduino as a byte stream over serial.
 * Arduino's are pretty dumb and cannot effectively run fft's, let alone while running neopixels
 * ENJOY!!!
 * 
 * Ilan aka EverydayEngineer
 * https://www.youtube.com/channel/UCNFzCN5QwFyOeDkdpx2_KvQ
 */
#include "simplified_neopixel.h"

#define NUM_COLS 30
#define NUM_ROWS 20

int Potl = A6;
int Potr = A4;

void setup() {  
  // Prepare the neopixel array for operation
  ledSetup();
  setBrightness(128);
  Clear();

  TIMSK0 = 0; // turn off timer0 for lower jitter 

  // Begin serial communication at some ridiculously high baud rate
  Serial.begin(1000000); 
  Serial.flush();
}


void loop(){
  uint8_t newBars[NUM_COLS] = {0};      // 30 byte array to hold incoming spectrum band data
  uint8_t oldGain = 0;                  // Holds old value of gain to be compared to current value
  uint8_t newGain = 0;                  // Holds up-to-date gain values to be compared to oldGain
  uint8_t pixels[3*(NUM_COLS*NUM_ROWS)];// Each neopixel requires 3 bytes of data corresponding to a green, red, and blue value (GRB)
                                        // Therefore we need an array of 3 times the total number of neopixels in the matrix
                                        // ***** Keep this variable local because arduino has much more local variable storage than global*****
  while(1){ // By putting the main loop in a while loop, there is more stability (according to the interwebs)
            // Haven't tested without it but most sample code that I saw had it so why not
      // Gain is controlled by the user via the left pot on the matrix
      // So this is the process by which we let processing know what's going on:
      newGain = (uint8_t)((1023 - analogRead(Potl))/4);     // This pot was wired backwards for convenience so account for that
      if(newGain > 10 + oldGain || newGain < oldGain-10)    // Sending gain constantly causes too much traffic on the serial line
      { 
        Serial.write(newGain);                              // So only send new gain values when an appreciable change has occured 
        oldGain = newGain;
      }

      // Now comes the good stuff:
      // Wait for new spectrum data to come in over the serial line
      if(Serial.available())                       
      { 
        Serial.readBytes(newBars, 30);  // Grab all 30 columns worth of data in one go
        setMatrix(newBars, pixels);     // Populate the pixel array with formatted column data
        show(pixels);                   // Push the new values to the matrix
      }
  }
}


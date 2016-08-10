// These values depend on which pin your string is connected to and what board you are using 
// More info on how to find these at http://www.arduino.cc/en/Reference/PortManipulation

#define PIXEL_PORT  PORTD  // Using digital port
#define PIXEL_DDR   DDRD   // Using digital port

#define PIXEL_PIN   6      // Pin the pixels are connected to
#define NUM_COLS    21     // Number of columns in our matrix
#define NUM_ROWS    10     // Number of rows in our matrix
#define COL_WIDTH   3      // Number of columns per frequency band

#define NUM_BANDS   ( NUM_COLS / COL_WIDTH )    // Number of frequency bands
#define NUM_PIXELS  ( NUM_COLS * NUM_ROWS )     // Number of total pixels

// These are the timing constraints taken mostly from the WS2812 datasheets 
#define T1H  800    // High width of a 1 bit in ns
#define T1L  450    // Low width of a 1 bit in ns

#define T0H  450    // High width of a 0 bit in ns
#define T0L  800    // Low width of a 0 bit in ns

#define RESET 5000    // Width of the low gap between bits to cause a frame to latch

// Here are some convience defines for using nanoseconds specs to generate actual CPU delays
#define NS_PER_SEC (1000000000L)
#define CYCLES_PER_SEC (F_CPU)
#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )
#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )
                        
// Actually send a bit to the string. We must to drop to asm to enusre that the complier does
// not reorder things and make it so the delay happens in the wrong place.

void sendBit( bool bitVal ) {
  
    if (  bitVal ) {        // 1 bit
      
        asm volatile (
          "sbi %[port], %[bit] \n\t"        // Set the output bit
          ".rept %[onCycles] \n\t"          // Execute NOPs to delay exactly the specified number of cycles
          "nop \n\t"
          ".endr \n\t"
          "cbi %[port], %[bit] \n\t"        // Clear the output bit
          ".rept %[offCycles] \n\t"         // Execute NOPs to delay exactly the specified number of cycles
          "nop \n\t"
          ".endr \n\t"
          ::
          [port]      "I" (_SFR_IO_ADDR(PIXEL_PORT)),
          [bit]       "I" (PIXEL_PIN),
          [onCycles]  "I" (NS_TO_CYCLES(T1H) - 2),  // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer
          [offCycles] "I" (NS_TO_CYCLES(T1L) - 2)   // Minimum interbit delay
        );
                                  
    } else {          // 0 bit

    // ********************//
    // TIMING MATTERS HERE //
    // ********************//

        asm volatile (
          "sbi %[port], %[bit] \n\t"      // Set the output bit
          ".rept %[onCycles] \n\t"        // Now timing actually matters. The 0-bit must be long enough to be detected but not too long or it will be a 1-bit
          "nop \n\t"                      // Execute NOPs to delay exactly the specified number of cycles
          ".endr \n\t"
          "cbi %[port], %[bit] \n\t"      // Clear the output bit
          ".rept %[offCycles] \n\t"       // Execute NOPs to delay exactly the specified number of cycles
          "nop \n\t"
          ".endr \n\t"
          ::
          [port]      "I" (_SFR_IO_ADDR(PIXEL_PORT)),
          [bit]       "I" (PIXEL_PIN),
          [onCycles]  "I" (NS_TO_CYCLES(T0H) - 2),
          [offCycles] "I" (NS_TO_CYCLES(T0L) - 2)
        );

    }
}  

  
void sendByte( uint8_t sByte ) {
    for( unsigned char bit = 0 ; bit < 8 ; bit++ ) {
      sendBit( bitRead( sByte , 7 ) );    // Neopixel wants bit in highest-to-lowest order
                                          // so send highest bit (bit #7 in an 8-bit byte since they start at 0)
      sByte <<= 1;                        // and then shift left so bit 6 moves into 7, 5 moves into 6, etc
    }           
} 


/************ledSetup()******************/
/// Description: declare which i/o pin will be sending data
///              * Must be called prior to using neopixels
/// Parameters: NONE
/// Return: NONE
/****************************************/
void ledSetup() {
  bitSet( PIXEL_DDR , PIXEL_PIN );
}

/************setBrightness()******************/
/// Description: changes brightness of neopixels
///              * If not called, defaults to 255
/// Parameters: value - value between 0-255 to signify fraction of 100% brightness
/// Return: NONE
/*********************************************/
uint8_t Brightness = 255;
void setBrightness(uint8_t value)  {
  Brightness = value;
  return;
}

// I'm using a look up table to select colors based on amplitude
// RGB values are used even though the Neopixel LEDs use GRB values
// because RGB values are easier to look up online.
// This is accounted for in the setMatrix function.

uint8_t GYR = {0x00, 0xff, 0x00, // Green
               0x00, 0xff, 0x00, // Green
               0x00, 0xff, 0x00, // Green
               0x00, 0xff, 0x00, // Green
               0x00, 0xff, 0xff, // Yellow
               0x00, 0xff, 0xff, // Yellow
               0x00, 0xff, 0xff, // Yellow
               0xff, 0x00, 0x00, // Red
               0xff, 0x00, 0x00, // Red
               0xff, 0x00, 0x00  // Red
};

uint8_t Rainbow = {0xbf, 0x00, 0xff, // Violet
               0x40, 0x00, 0xff, // Blue
               0x00, 0x40, 0xff, // Lighter Blue
               0x00, 0xbf, 0xff, // Cyan
               0x00, 0xff, 0xbf, // Seafoam
               0x00, 0xff, 0x40, // Light Green
               0x40, 0xff, 0x00, // Green
               0xbf, 0xff, 0x00, // Greenish-Yellow
               0xff, 0xbf, 0x00, // Orange
               0xff, 0x40, 0x00  // Redish-Orange
};



/************setMatrix()******************/
/// Description: Takes a <NUM_BANDS>-long byte-array consisting of
///              amplitudes for each column of the matrix, and 
///              populates the pixels array accordingly
/// Parameters: Bars - byte array of new amplitudes
///             pixels - byte array that will hold data for each pixel
/// Return: NONE
/*****************************************/
void setMatrix(uint8_t * Bars, uint8_t pixels[])  {

  uint16_t k = 0;
  for( uint8_t i = 0; i < NUM_BANDS; i++ ) {
    for ( uint8_t j = 0; j < NUM_ROWS*3; j+=3 ) {
      if( (i % 2 ) == 0 ) {  // Even numbered band
      
        k = i * 90;
        if(j <= Bars[i]) {

          uint8_t R = GYR[j/3];    // Get RGB values from look up table
	  uint8_t G = GYR[(j/3)+1];
	  uint8_t B = GYR[(j/3)+2];
	

          pixels[k]   = G;     // G value for column 1 of current band
          pixels[k+1] = R;     // R value for column 1 of current band
          pixels[k+2] = B;     // B value for column 1 of current band

          pixels[k+57-j] = G;  // G value for column 2 of current band
          pixels[k+58-j] = R;  // R value for column 2 of current band
          pixels[k+59-j] = B;  // B value for column 2 of current band

          pixels[k+60+j] = G;  // G value for column 3 of current band
          pixels[k+61+j] = R;  // R value for column 3 of current band
          pixels[k+62+j] = B;  // B value for column 3 of current band

        } else {

          pixels[k]   = 0x00;     // G value for column 1 of current band
          pixels[k+1] = 0x00;     // R value for column 1 of current band
          pixels[k+2] = 0x00;     // B value for column 1 of current band

          pixels[k+57-j] = 0x00;  // G value for column 2 of current band
          pixels[k+58-j] = 0x00;  // R value for column 2 of current band
          pixels[k+59-j] = 0x00;  // B value for column 2 of current band

          pixels[k+60+j] = 0x00;  // G value for column 3 of current band
          pixels[k+61+j] = 0x00;  // R value for column 3 of current band
          pixels[k+62+j] = 0x00;  // B value for column 3 of current band

        }

      } else {  // Odd numbered band
        
	k = (i * 90) + 27;

        if(j <= Bars[i]) {

          uint8_t R = GYR[j/3];    // Get RGB values from look up table
	  uint8_t G = GYR[(j/3)+1];
	  uint8_t B = GYR[(j/3)+2];
		
          pixels[k-j]   = G;
          pixels[k+1-j] = R;
          pixels[k+2-j] = B;
        
          pixels[k+3+j] = G;
          pixels[k+4+j] = R;
          pixels[k+5+j] = B;
        
          pixels[k+60-j] = G;
          pixels[k+61-j] = R;
          pixels[k+62-j] = B;
        
        } else {

          pixels[k-j]   = 0x00;
          pixels[k+1-j] = 0x00;
          pixels[k+2-j] = 0x00;
        
          pixels[k+3+j] = 0x00;
          pixels[k+4+j] = 0x00;
          pixels[k+5+j] = 0x00;
        
          pixels[k+60-j] = 0x00;
          pixels[k+61-j] = 0x00;
          pixels[k+62-j] = 0x00;
        }
        
      }
    }
  }

  return;
}

/************Clear()*********************/
/// Description: Send all 0s to the matrix
/// Parameters: NONE
/// Return: NONE
/****************************************/
void Clear()  {
  cli();                                // Turn off all interrupts while we send data to the matrix
  for(int i = 0; i < NUM_PIXELS; i++) {
    sendByte(0);
    sendByte(0);
    sendByte(0);
  }
  sei();                                // All of the data is sent so we can turn back on interrupts
  // By sending creating a specifically timed delay, the neopixels will know that we are done sending data
  _delay_us( (RESET / 1000UL) + 1);       // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}


/************show()******************/
/// Description: "Refresh" the neopixel matrix with
///               updated values 
///               * Should be called once the pixel array
///                 is updated and ready to be pushed
/// Parameters: pixels - the byte array containing values for all the pixels
/// Return: NONE
/************************************/
void show(uint8_t * pixels) {
  cli();                                              // Turn off all interrupts while we send new data
  for(int i = 0; i < NUM_PIXELS*3; i++) {
    if(pixels[i] != 0 && Brightness != 0)  {  
      sendByte(((Brightness * pixels[i]) >> 8) + 1);  // This is when we take pixel brightness into account
    }
    else  {
      sendByte(0);
    }
  }
  sei();                                              // All data is sent so we can turn interrupts back on
  // By sending creating a specifically timed delay, the neopixels will know that we are done sending data
  _delay_us( (RESET / 1000UL) + 1);       // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
  return;
}


// These values depend on which pin your string is connected to and what board you are using 
// More info on how to find these at http://www.arduino.cc/en/Reference/PortManipulation

#define PIXEL_PORT  PORTD  // Port of the pin the pixels are connected to
#define PIXEL_DDR   DDRD   // Port of the pin the pixels are connected to

#define PIXEL_PIN   6      // Bit of the pin the pixels are connected to
#define NUM_COLS    21     // Number of columns in our neopixel matrix
#define NUM_ROWS    10     // Number of rows in our neopixel matrix
#define COL_WIDTH   3

#define NUM_BANDS   ( NUM_COLS / COL_WIDTH )
#define NUM_PIXELS  ( NUM_COLS * NUM_ROWS )

// These are the timing constraints taken mostly from the WS2812 datasheets 
#define T1H  800    // Width of a 1 bit in ns
#define T1L  450    // Width of a 1 bit in ns

#define T0H  450    // Width of a 0 bit in ns
#define T0L  800    // Width of a 0 bit in ns

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
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]      "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]       "I" (PIXEL_PIN),
      [onCycles]  "I" (NS_TO_CYCLES(T1H) - 2),    // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
      [offCycles] "I" (NS_TO_CYCLES(T1L) - 2)   // Minimum interbit delay. Note that we probably don't need this at all since the loop overhead will be enough, but here for correctness

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

/*
  The following three functions are the public API:
  
  ledSetup() - set up the pin that is connected to the string. Call once at the begining of the program.  
  setBrightness(byte) - changes the brightness of the neopixels (0(low)->255(high))
  setMatrix(*byte, byte[]) - Converts a byte array of amplitudes into appropriate pixel orientation for matrix
  clear() - writes 0's to all pixels to clear the matrix
  show(*byte) - push updated pixel data to the matrix 
  
*/


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

/************setMatrix()******************/
/// Description: Takes a NUM_COLS-long byte-array consisting of
///              amplitudes for each column of the matrix, and 
///              populates the pixels array accordingly
/// Parameters: Bars - byte array of new amplitudes
///             pixels - byte array that will hold data for each pixel
/// Return: NONE
/*****************************************/
void setMatrix(uint8_t * Bars, uint8_t pixels[])  {
  uint8_t k = 0;
  for( uint8_t i = 0; i < NUM_BANDS; i++) {
    k = i * 30;
    if( (i % 2 ) == 0 ) {  // Even numbered band
      for( uint8_t j = 0; j < NUM_ROWS; j++) {
        
        if(j < Bars[i]) {
          pixels[k] = 0x0;        // G value for column 1 of current band
          pixels[k+1] = 0x64;     // R value for column 1 of current band
          pixels[k+2] = 0x0;      // B value for column 1 of current band

          pixels[k+19-j] = 0x0;   // G value for column 2 of current band
          pixels[k+20-j] = 0x64;  // R value for column 2 of current band
          pixels[k+21-j] = 0x0;   // B value for column 2 of current band

          pixels[k+66+j] = 0x0;   // G value for column 3 of current band
          pixels[k+67+j] = 0x64;  // R value for column 3 of current band
          pixels[k+68+j] = 0x0;   // B value for column 3 of current band
        } else {
          pixels
          pixels
          pixels
          
          pixels
          pixels
          pixels
          
          pixels
          pixels
          pixels
        }
      }
    } else {  // Odd numbered band
      for( uint8_t j = 0; j < NUM_ROWS; j++) {

        if(j < Bars[i]) {
        pixels
        pixels
        pixels
        
        pixels
        pixels
        pixels
        
        pixels
        pixels
        pixels
        
        } else {
          pixels[WHATEVER] = 0;
          pixels[WHATEVER+1] = 0;
          pixels[WHATEVER+2] = 0;
          
          pixels
          pixels
          pixels
          
          pixels
          pixels
          pixels
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

// Just wait long enough without sending any bits to cause the pixels to latch and display the last sent frame

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


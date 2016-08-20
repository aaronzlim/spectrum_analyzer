// Testing the MSGEQ7 graphic equalizer display filter

#define IN     A5  // Port F bit 0
#define STRB   12  // Port D bit 6
#define RST    4   // Port D bit 4

#define PORT PORTD // PD6 and PD12 use PORTD
#define DDR  DDRD

#define STRB_TOGGLE 6
#define RST_TOGGLE  4

#define STRB_WIDTH   20 // Strobe high width 18us min
#define SETTLE       40 // Settling time of 36us min
#define RST_DELAY    80 // Delay after reset of 72us min

#define NUM_FILTERS 7 // This chip has 7 bandpass filters

uint8_t filter = 0; // Initialize iterator
uint16_t results[NUM_FILTERS] = {}; // Array to store results

void setup() {
  Serial.begin(9600); // Setup serial communication
  while(!Serial);     // Wait for serial to be recognized

  bitClear(DDRF, 0);  // Set A5 as input
  
  bitSet(DDR, RST_TOGGLE);  // set reset pin as output
  bitSet(DDR, STRB_TOGGLE); // set strobe pin as output

  bitSet(PORT, STRB_TOGGLE);  // Initialize
  bitSet(PORT, RST_TOGGLE);   // Reset mux
  _delay_us(1);
  bitClear(PORT, RST_TOGGLE);
  _delay_us(RST_DELAY);
}


void loop() {

  for(filter = 0; filter < NUM_FILTERS; filter++) {
    bitClear(PORT, STRB_TOGGLE);      // Clear strobe
    _delay_us(SETTLE);                // Wait for data to settle
    results[filter] = analogRead(IN); // Record result
    bitSet(PORT, STRB_TOGGLE);        // Set strobe (clears output)
    _delay_us(STRB_WIDTH);            // Wait for mux to switch filters
  }

  Serial.print("---------- BANDS ----------\n");
  for(filter = 0; filter < NUM_FILTERS; filter++) {
    Serial.print(results[filter]); Serial.print(" ");
  }
  Serial.print("\n\n");

  delay(500);
 
}




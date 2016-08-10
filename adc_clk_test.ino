/*
    See the Atmel ATmega32U4 datasheet for more info
*/

#define PIN 2    // Input pin
#define N   100  // Number of iterations

// Arrays to save results
unsigned long start_times[N];
unsigned long stop_times[N];
unsigned long values[N];

// Define ADC prescaler values
const unsigned char PS_16  = (1 << ADPS2);
const unsigned char PS_32  = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64  = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

void setup() {

    // Setup serial communication
    Serial.begin(9600);
    pinMode(PIN, INPUT);

    ADCSRA &= ~PS_128;    // Remove bits set by the Arduino Library
    
    // Choose a prescaler
    ADCSRA |= PS_32;      // Should get 38kHz sample rate
}


void loop() {

    uint8_t i = 0;

    // Collect analog read data
    for (i = 0; i < N; i++) {
        start_times[i] = micros();
        values[i] = analogRead(2);
        stop_times[i] = micros();
    }
    
    // Print the results
    Serial.println("\n\n--- Results ---\n");
    for(i = 0; i < N; i++) {
        Serial.print(values[i]);
        Serial.print(" del_t = ");
        Serial.print(stop_time[i] - start_time[i]);
        Serial.print(" us\n");
    }
    delay(10000);
}

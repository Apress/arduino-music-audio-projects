/*
fft_adc_serial.pde - Modified by Mike Cook
38.195KHz sample rate
guest openmusiclabs.com 7.7.14
example sketch for testing the fft library.
it takes in data on ADC0 (Analog0) and processes them
with the fft. the data is sent out over the serial
port at 250kb.   
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
int timer0;
void setup() {
  Serial.begin(250000); // use the serial port
  timer0 = TIMSK0;
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  pinMode(8,INPUT_PULLUP); // for freezing the display
}

void loop() {
  while(1) { // reduces jitter
    TIMSK0 = 0; // turn off timer0 for lower jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft in log form
    fft_mag_octave();
    sei(); // enable interrupts
    TIMSK0 = timer0; // restart the timer
    // send out the bins to the plotter
    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
      for(byte j=0; j<3; j++){ // each bin three times
     Serial.println(fft_log_out[i]); // send out the data
      }
    }
      for(byte j=0; j<116; j++) Serial.println(0);
     // while(digitalRead(8)) { } // hold until this pin is low
     delay(2000);
  }
}

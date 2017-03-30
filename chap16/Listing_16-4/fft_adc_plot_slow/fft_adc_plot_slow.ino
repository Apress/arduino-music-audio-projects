/*
fft_adc_plot_slow - by Mike Cook
Sampling at 8.88KHz
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library

int timer0;
void setup() {
  Serial.begin(250000); // use the serial port
  timer0 = TIMSK0; // save normal timer 0 state
  TIMSK0 = 0;
  pinMode(8,INPUT_PULLUP); // for freezing the display
  // pinMode(2,OUTPUT); // for monitering sample rate
}

void loop() {
  int k;
  while(1) { // reduces jitter
    TIMSK0 = 0; // turn off timer0 for lower jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    // PORTD |= 0x4; // set pin 2 high to time 256 samples
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples      
      k = (analogRead(0) - 0x0200 )<< 6;
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    // PORTD ^= 0x4; // clear pin 2 high to time 256 samples
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft in log form
    sei(); // enable interrupts
    TIMSK0 = timer0; // restart the timer
    // send out the bins to the plotter
    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
      for(byte j=0; j<3; j++){ // each bin three times
     Serial.println(fft_log_out[i]); // send out the data
    //  Serial.println(fft_oct_out[i]); // send out the data
      }
    }
      for(byte j=0; j<116; j++) Serial.println(0);
     // while(digitalRead(8)) { } // hold until this pin is low
     delay(2000);
  }
}

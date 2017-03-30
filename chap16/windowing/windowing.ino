/*
Windowing - Mike Cook
Visulisation of the Hann window function
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
int displayCount = 499;

int timer0;
void setup() {
  int k;
  Serial.begin(250000); // use the serial port
    float count = 0.0;
    float increment= 360.0 / (255.0/ 13.0);
    for (int i = 0 ; i < 512 ; i += 2) { // generate 256 samples
      count += increment;
      k = (int)(4960.0 * sin(count / 57.2957795 ));
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    // fft_window(); // window the data for better frequency response
    // send out the bins to the plotter
    for (int i = 0 ; i < 512 ; i += 2){ 
      for(byte j=0; j<2; j++){ // each bin twice
        Serial.println(fft_input[i]); // send out the data
        displayCount--;
      }
    }
        while(displayCount > 0){ // shift to end
    Serial.println(0);
    displayCount--; 
    }

  }

void loop() {
}

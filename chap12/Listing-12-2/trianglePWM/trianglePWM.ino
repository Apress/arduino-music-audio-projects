 // Triangle wave PWM output
 volatile int sample =0;
 volatile int increment =  0x2; // sets frequency
 volatile boolean hush = false;
 volatile boolean slope = false;
 const byte hushPin = 2;

ISR(TIMER2_COMPB_vect){  // Generate the next sample and send it to the A/D     
   if(!hush) { // if playing a note then output the next sample
      if (slope) sample += increment; else sample -= increment;
      if (sample < 0){ // if sample has peaked
        slope = !slope; // reverse direction
        // do a double increment to send it in the right direction
        if (slope) sample += (increment<<1); else sample -= (increment<<1);
      }
      OCR1AL = sample >>7 ; // use sample to change PWM duty cycle
        }
}

void setPWMtimer(){
// Set timer1 for 8-bit fast PWM output to use as our analogue output
 TCCR1B = _BV(CS10); // Set prescaler to full 16MHz for 2 cycles per sample
 TCCR1A |= _BV(COM1A1); // Pin low when TCNT1=OCR1A
 TCCR1A |= _BV(WGM10); // Use 8-bit fast PWM mode
 TCCR1B |= _BV(WGM12);  
}

void setSampleTimer(){  // sets timer 2 going at the output sample rate
  TCCR2A = _BV(WGM21) | _BV(WGM20); // Disable output on Pin 11 and Pin 3
  TCCR2B = _BV(WGM22) | _BV(CS22);
  OCR2A = 60; // defines the interval to trigger the sample generation - 30uS or 33.3KHz
  TCCR2B = TCCR2B & 0b00111000 | 0x2; // select a prescale value of 8:1 of the system clock
  TIMSK2 = _BV(OCIE2B); // Output Compare Match B Interrupt Enable
}

void setup() {
  pinMode(9, OUTPUT); // Make timer’s PWM pin an output
  pinMode(hushPin,INPUT_PULLUP);
  setSampleTimer();
  setPWMtimer();
}

void loop() {
  // gate tone
  if(digitalRead(hushPin)) hush=false; else hush=true;
}

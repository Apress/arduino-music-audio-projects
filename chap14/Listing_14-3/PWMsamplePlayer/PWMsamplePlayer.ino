// PWM sample player - using Timer 1's PWM as output
// on pin 9 with internal flash memory holding the sample
// By Mike Cook Nov 2015

 #include "no.h"  // tab containing samples saying "No"
 #include "yes.h" // tab containing samples saying "Yes"
 
 volatile int sampleCount =0;
 volatile int sampleLimit =0; // number of bytes in the sample
 volatile const byte *sample;
 volatile boolean playing = false;
 const byte play1Pin = 2, play2Pin = 3;

ISR(TIMER2_COMPB_vect){  // Generate the next sample and send it to the A/D     
  sampleCount ++;
  if(sampleCount >= sampleLimit){ // have we finished
     TIMSK2 &= ~_BV(OCIE2B); // interrupts off
     OCR1AL = 0x80; // leave PWM at mid point
     playing=false;
     return;
   }
   OCR1AL = pgm_read_byte_near(&sample[sampleCount]); // use sample to change PWM duty cycle
 }

void setPWMtimer(){
// Set timer1 for 8-bit fast PWM output to use as our analogue output
 TCCR1B = _BV(CS10); // Set prescaler to full 16MHz
 TCCR1A |= _BV(COM1A1); // Pin low when TCNT1=OCR1A
 TCCR1A |= _BV(WGM10); // Use 8-bit fast PWM mode
 TCCR1B |= _BV(WGM12); 
 OCR1AL = 0x80; // start PWM going at half maximum 
}

void setSampleTimer(){  // sets timer 2 going at the output sample rate
  TCCR2A = _BV(WGM21) | _BV(WGM20); // Disable output on Pin 11 and Pin 3
  TCCR2B = _BV(WGM22) | _BV(CS22);
  OCR2A = 250; // defines the interval to trigger the sample generation - 125uS or 8.0KHz
  TCCR2B = TCCR2B & 0b00111000 | 0x2; // select a prescale value of 8:1 of the system clock
  TIMSK2 = _BV(OCIE2B); // Output Compare Match B Interrupt Enable
}

void setup() {
  pinMode(9, OUTPUT); // Make timer’s PWM pin an output
  pinMode(play1Pin,INPUT_PULLUP);
  pinMode(play2Pin,INPUT_PULLUP);
  setSampleTimer();
  setPWMtimer();
 }

void loop() { // play samples on grounding pins
  if(!digitalRead(play1Pin)){
    playSample(&yes[0],true); // start playing and then return
  }
  if(!digitalRead(play2Pin)){
    playSample(&no[0],false); // only return when finished playing sample
  }
}

void playSample(const byte *toPlay, boolean aysync){
   TIMSK2 &= ~_BV(OCIE2B); // stop any playing
   sample = toPlay;
   sampleLimit = pgm_read_byte_near(&toPlay[0]) | (pgm_read_byte_near(&toPlay[1]) << 8);
   sampleCount =1;
   playing = true;
   TIMSK2 = _BV(OCIE2B); // allow interrupts
   if( !aysync ) { // play sample till the end if not aysync
        while(playing) { } // hold until finished
   }
}


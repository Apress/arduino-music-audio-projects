// Poly Tone Woops - Mike Cook
  volatile byte tone1period = 1, tone2period = 1;
 volatile byte tone1restore = 41, tone2restore = 41;
 volatile unsigned int tcnt2;
 
ISR(TIMER2_OVF_vect){  // Interrupt service routine to generate the tones
  TCNT2 = tcnt2 + TCNT2; // reload counter
  tone1period--; 
  if(tone1period < 1) { // time to toggle the first pin
    tone1period = tone1restore;
    PORTB ^= _BV(5); // toggle pin 13, PB5 direct port addressing
  }  
  tone2period--;
  if(tone2period < 1) { // time to toggle the second pin
    tone2period = tone2restore;
    PORTB ^= _BV(4); // toggle pin 12, PB4 direct port addressing
  }
}

void setUpTimer(){  // sets the timer going at the decrement rate
TIMSK2 &= ~_BV(TOIE2);  // Disable the timer overflow interrupt
TCCR2A &= ~(_BV(WGM21) | _BV(WGM20)); // Configure timer2 in normal mode
TCCR2B &= ~_BV(WGM22);
ASSR &= ~_BV(AS2); // Select clock source: internal I/O clock
TIMSK2 &= ~_BV(OCIE2A); //Disable Compare Match A interrupt (only overflow)
TCCR2B |= _BV(CS22)  | _BV(CS20); // Set bits
TCCR2B = (TCCR2B & 0b00111000) | 0x1; // select a prescale value of 1:1
tcnt2 = 96 + 5; // give 10uS interrupt rate + adjustment 
TCNT2 = tcnt2; // pre load the value into the timer
}

void setup(){
  setUpTimer();
  pinMode(13,OUTPUT); // enable the pins you want to use as tone outputs
  pinMode(12,OUTPUT);
  TIMSK2 |= _BV(TOIE2);  // tone on
}

void loop() {
  // raise tone 2 and drop tone 1 at diffrent rates
  tone1restore += 10; // lower tone 1
  if(tone1restore > 240) tone1restore = 20; // back to initial position
  tone2restore -= 28; // increase tone 2
  if(tone2restore < 20) tone2restore = 240; // back to initial position
  delay(300); // speed of note changes
}



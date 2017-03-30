// DTMF telephone numbers - Mike Cook
 volatile byte tone1period = 1, tone2period = 1;
 volatile byte tone1restore = 41, tone2restore = 41;
 volatile unsigned int tcnt2;
 int toneOn = 90, toneOff = 100;
 
 byte rowTone [] = { 71, 65, 59, 53 };
 byte colTone [] = { 41, 37, 34, 31 };
 byte  pad[4][4] = { '1','2','3','A', 
                     '4','5','6','B',
                     '7','8','9','C',
                     '*','0','#','D' }; 
 
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
  tone1restore = 30;
  tone2restore = 53;
  sendTones("0123 45678#"); // phone number
}

void loop() {
}

void sendTones(char *number){
  byte digit = 1, point = 0;
  while( digit != 0){
    digit = number[point];
    if( digit !=0) sendTone(digit);
    point++;
  }
}

void sendTone(byte key){  
  boolean found = false;
  int i=0,j=0;
  while(j < 4 && !found){
   i=0;
   while(i<4 && !found){
    if(pad[j][i] == key) {
       tone1restore = rowTone[j];
       tone2restore = colTone[i];
       tone1period = 1;
       tone2period = 1;
       // generate tones
         TIMSK2 |= _BV(TOIE2);  // tone on
         delay(toneOn); // length of tone on
         TIMSK2 &= ~_BV(TOIE2);  // tone off
         delay(toneOff); // length of gap between tones
        found = true;
    }
    i++; // move on row
    }
   j++; // move on coloum
  }
  if(!found) {
    delay(toneOff); // small gap for unknown digits
  }
}



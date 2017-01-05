 // Wave table output D/A output
 #include <SPI.h>

#define CS_ADC_BAR 10
#define AD_LATCH 6
// temp
#define CS1_BAR 9
#define CS2_BAR 8
#define Hold_BAR 7

 volatile unsigned int sampleIndex =0;
 volatile int increment =  0x100; // sets frequency
 volatile boolean hush = false;
 const byte hushPin = 2;
 int waveTable [256];

ISR(TIMER2_COMPB_vect){  // Look up the next sample and send it to the A/D     
   if(!hush) { // if playing a note then output the next sample
      sampleIndex += increment; // increment sample    
      ADwrite(waveTable[sampleIndex >> 8])  ; // output to A/D
   }
}

void setSampleTimer(){  // sets timer 2 going at the output sample rate
  TCCR2A = _BV(WGM21) | _BV(WGM20); // Disable output on Pin 11 and Pin 3
  TCCR2B = _BV(WGM22) | _BV(CS22);
  OCR2A = 60; // defines the interval to trigger the sample generation - 30uS or 33.3KHz
  TCCR2B = TCCR2B & 0b00111000 | 0x2; // select a prescale value of 8:1 of the system clock
  TIMSK2 = _BV(OCIE2B); // Output Compare Match B Interrupt Enable
}

void setup() {
  // temp
   pinMode( CS1_BAR, OUTPUT);
  digitalWrite( CS1_BAR, HIGH);
  pinMode( CS2_BAR, OUTPUT);
  digitalWrite( CS2_BAR, HIGH);
  pinMode( CS_ADC_BAR, OUTPUT);
  digitalWrite(CS_ADC_BAR, HIGH);
  pinMode( AD_LATCH, OUTPUT);
  digitalWrite(AD_LATCH, HIGH);
  pinMode( Hold_BAR, OUTPUT);
  digitalWrite( Hold_BAR, HIGH);

  // initilise control pins for A/D & SPI  
  pinMode( CS_ADC_BAR, OUTPUT);
  digitalWrite(CS_ADC_BAR, HIGH);
  pinMode( AD_LATCH, OUTPUT);
  digitalWrite(AD_LATCH, HIGH);
  pinMode(hushPin,INPUT_PULLUP);
  SPI.begin();
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV2); // maximum clock speed
  generateTable(); // calculate the lookup table
  setSampleTimer();
}

void loop() {
  // gate tone
  if(digitalRead(hushPin)) hush=false; else hush=true;
}

void generateTable(){
  const float pi = 3.1415;
  float angle;
  int sample = 2048;
  for(int i =0;i<128;i++){ // first part of the wave table is a sin
    angle = ((2.0 * pi)) / (128.0 / (float)i);
    waveTable[i] = (int)(2047.0 + 2047.0 * sin(angle));
  }
  for(int i =128;i<256;i++){ // second part of the wave table is a saw
   waveTable[i] = sample;
   sample = (sample - 64) & 0xFFF; // keep to 12 bits
  }
}

void ADwrite(int data){
 // digitalWrite(CS_ADC_BAR, LOW); // replace by below
  PORTB &= ~0x4;
  SPI.transfer(((data >> 8) & 0x0f) | 0x70);
  SPI.transfer(data & 0xff);
  //digitalWrite(CS_ADC_BAR, HIGH); // replace by below
  PORTB |= 0x4;
  //digitalWrite(AD_LATCH, LOW); // replace by below
  PORTD &= ~0x40;
  //digitalWrite(AD_LATCH, HIGH); // replace by below
  PORTD |= 0x40;
}

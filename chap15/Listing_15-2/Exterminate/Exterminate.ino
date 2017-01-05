/* `Exterminate - Mike Cook
modulate a smample with a 30 Hz triangle wave
*/
#include <SPI.h>

#define CS1_BAR 9
#define CS2_BAR 8
#define CS_ADC_BAR 10
#define AD_LATCH 6
#define Hold_BAR 7

void setup() {
  // initilise control pins for SRAM  
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
  
   SPI.begin();
   SPI.setDataMode(SPI_MODE3);
   SPI.setClockDivider(SPI_CLOCK_DIV2); // 8MHz clock this is the maximum clock speed
   // set up fast ADC mode
   ADCSRA = (ADCSRA & 0xf8) | 0x04; // set 16 times division
   pinMode(2,OUTPUT); // for monitering sample rate
 }
 
float mod = 0.5;
float increment =  0.0035;
int sampleIn, sampleOut;
int midPoint = 2048; // 2048 for 12 bit 512 for 10 bits

 void loop(){
   // make triangle wave
    mod += increment;
    if(mod > 0.90 || mod < 0.02) {
      increment = -increment;
      mod += increment;
    }
  sampleIn = (analogRead(0) <<2 ) -midPoint; 
  sampleOut = (float)sampleIn * mod;
  ADwrite(sampleOut + midPoint);
  PORTD ^= 0x4; // toggle pin each sample to check sample rate
   }
   
void ADwrite(int data){
 // digitalWrite(CS_ADC_BAR, LOW);  - direct addressing below
  PORTB &= ~0x4;
  SPI.transfer(((data >> 8) & 0x0f) | 0x70);
  SPI.transfer(data & 0xff);
  //digitalWrite(CS_ADC_BAR, HIGH);  - direct addressing below
  PORTB |= 0x4;
  //digitalWrite(AD_LATCH, LOW);  - direct addressing below
  PORTD &= ~0x40;
  //digitalWrite(AD_LATCH, HIGH);  - direct addressing below
  PORTD |= 0x40;
}


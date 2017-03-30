/* Audio effects - Mike Cook
A 16 input switch decide the delay / effect
*/

/* the memory chip is 128K bytes - address from 0x0 to 0x1FFFF ( decimal 131071 )
and therefore 64K samples long 
*/
#include <SPI.h>

#define CS1_BAR 9
#define CS2_BAR 8
#define CS_ADC_BAR 10
#define AD_LATCH 6
#define Hold_BAR 7

// All buffer sizes are in samples (2 bytes) not in bytes
//long int bufferSize = 30000;  // absoloute size of circular buffer for 2 memory chips
long int bufferSize = 0xFFFF;  // maximum address of circular buffer for 1 memory chip
long int bufferOffset = 5000; // distance between input and output points
long int bufferOffset2, bufferOffset3, bufferOffset4;
byte program = 0;

void setup()
{
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
   setChipMode(0x40, CS1_BAR); // sequential mode for chip 1
   // set up fast ADC mode
   ADCSRA = (ADCSRA & 0xf8) | 0x04; // set 16 times division
   // set input pull up pins for HEX mode switch
   for( int i= 16; i<20; i++){
     pinMode(i, INPUT_PULLUP);
   }
   // read in the HEX (or BCD) mode switch
   for(int i=19; i>15; i--){  
      program = program << 1; // move a space for the next bit   
      program |= digitalRead(i) & 0x1; // add next bit 
   }
   program ^= 0x0f; // invert it 
   setUpMyBuffers(program); // initilise buffer pointers
   // blank memory for initial buffer to avoid noise in phones on power up
   if( program >7) blankMemory(bufferOffset4); else blankMemory(bufferOffset); 
}

void loop(){ 
 if(program < 5) basicDelay(); // all simple delays of differing length
 if(program == 5) pitchUp();
 if(program == 6) pitchDown();
 if(program == 7) reverse();
 if(program == 8) echo4();  // three echos and a repeat
 if(program == 9) echo4();
}

void echo4(){  // loop time 95.2uS - sample rate 10.5KHz
  // set up the initial position of the buffer pointers
   static int sample;
   static long bufferIn=bufferSize, bufferOut1=bufferSize - bufferOffset; 
   static long bufferOut2=bufferSize - bufferOffset2, bufferOut3=bufferSize - bufferOffset3;
   static long bufferOut4=bufferSize - bufferOffset4;
 while(1){
    sample = analogRead(0);
    saveSample(bufferIn, sample);
    sample = fetchSample(bufferOut1); 
    sample += fetchSample(bufferOut2); 
    sample += fetchSample(bufferOut3); 
    sample += fetchSample(bufferOut4);
    sample = sample >> 2; // so as to keep the gain down to 1 to avoid build up of noise
    ADwrite(sample); // output sample
    // adjust the buffer pointers
    bufferIn++;
    if(bufferIn > bufferSize) bufferIn=0;
    bufferOut1++;
    if(bufferOut1 > bufferSize) bufferOut1=0;
    bufferOut2++;
    if(bufferOut2 > bufferSize) bufferOut2=0;
    bufferOut3++;
    if(bufferOut3 > bufferSize) bufferOut3=0;
    bufferOut4++;
    if(bufferOut4 > bufferSize) bufferOut4=0;
  }
}

void reverse(){
 static  long bufferIn=0, bufferOut=0, sample; 
while(1){
  sample = analogRead(0);
  saveSample(bufferIn, sample);
  sample = fetchSample(bufferOut); 
  ADwrite(sample);
  bufferIn++;
  if(bufferIn > bufferOffset) bufferIn=0;
  bufferOut--;
  if(bufferOut < 0 ) bufferOut=bufferOffset; 
  } 
}

void pitchDown(){
 static unsigned int bufferIn=0, bufferOut=0, sample; 
while(1){
  sample = analogRead(0);
  saveSample(bufferIn, sample);
  bufferIn++;
  if(bufferIn > bufferOffset) bufferIn=0;
  saveSample(bufferIn, sample);
  bufferIn++;
  if(bufferIn > bufferOffset) bufferIn=0;
  sample = fetchSample(bufferOut); 
  ADwrite(sample);
  bufferOut++;
  if(bufferOut > bufferOffset) bufferOut=0; 
  }
  
}
void pitchUp(){
 static unsigned int bufferIn=0, bufferOut=0, sample; 
while(1){
  sample = analogRead(0);
  saveSample(bufferIn, sample);
  sample = fetchSample(bufferOut); 
  ADwrite(sample);
  bufferIn++;
  if(bufferIn > bufferOffset) bufferIn=0;
  bufferOut +=2;
  if(bufferOut > bufferOffset) bufferOut=0; 
  }  
}

void basicDelay(){  // loop time 48.2uS - sample rate 20.83 KHz
 static unsigned int bufferIn=bufferSize, bufferOut=bufferSize - bufferOffset, sample; 
while(1){
  sample = analogRead(0);
  saveSample(bufferIn, sample);
  sample = fetchSample(bufferOut); 
  ADwrite(sample);
  bufferIn++;
  if(bufferIn > bufferSize) bufferIn=0;
  bufferOut++;
  if(bufferOut > bufferSize) bufferOut=0; 
  }
  
}

void setUpMyBuffers(byte p){
  bufferSize = 30000;
  switch(p) { // depending on program mode initilise the buffer pointers
    case 0 :
        bufferOffset = 1000; // samples between in and out 0.05 seconds
    break;
    case 1 :
        bufferOffset = 3000; //  samples between in and out 0.15 seconds
    break;
    case 2 :
       bufferOffset = 5000; // samples between in and out 0.25 seconds
    break;
    case 3 :
       bufferOffset = 10000; // samples between in and out 0.5 seconds
    break;
    case 4 :
       bufferOffset = 20000; // samples between in and out 1 second
    break;
    case 5 :
       bufferOffset = 1000; // size of buffer for pitch up
    break;
    case 6 :
        bufferOffset = 1000; // size of buffer for pitch down
    break;
    case 7 :
        bufferOffset = 32000; // size of buffer for reverse
    break;
    case 8 :
        // bufferSize = 100000;
        bufferOffset = 3000; // distance of input pointer to first echo 0.3 seconds
        bufferOffset2 = 6000; // distance of input pointer to second echo 0.58 seconds
        bufferOffset3 = 9000; // distance of input pointer to third echo  0.86 seconds
        bufferOffset4 = 22000; // distance of input pointer to fourth echo 2 seconds
     break;
    case 9 :
        // bufferSize = 100000;
        bufferOffset =  12000;   // distance of input pointer to first echo
        bufferOffset2 = 24000;  // distance of input pointer to second echo
        bufferOffset3 = 36000;  // distance of input pointer to third echo
        bufferOffset4 = 48000; // distance of input pointer to fourth echo 5.1 seconds
     break;

    default :
    bufferOffset = 1000;
  }  
}


void setChipMode(int value, int where){
  digitalWrite( where, LOW); // CE pin
  SPI.transfer(0xff); // reset any double or quad mode
  digitalWrite( where, HIGH); // CE pin
  delay(2);
  digitalWrite( where, LOW); // CE pin
  SPI.transfer(0x01); // write to mode register
  SPI.transfer(value); // the value passed into it
  digitalWrite( where, HIGH); // CE pin
}

int fetchSample(long address){ // given sample address
  int data;
  address = address << 1; // make it into byte address
  //digitalWrite(CS1_BAR, LOW); // CE pin  - direct addressing below
  PORTB &= ~0x02;
  SPI.transfer(0x03);  // read data from memory in sequence mode
  SPI.transfer((address>>16) & 0xff); // write 3 byte address most significant first 
  SPI.transfer((address>>8) & 0xff);
  SPI.transfer(address & 0xff);
  data = SPI.transfer(0) << 8;
  data |= SPI.transfer(0);
  //digitalWrite(CS1_BAR, HIGH); // CE pin  - direct addressing below
  PORTB |= 0x02;
 return data; 
}

void blankMemory(long bufferLen){
  int blank = analogRead(0); // take the current input level and fill memory with it
  for(long memPoint = bufferSize - bufferLen; memPoint <= bufferSize; memPoint++){
    saveSample(memPoint,blank);
  }
}

int saveSample(long address, int data){ // given sample address
  address = address << 1; // make it into byte address
  //digitalWrite(CS1_BAR, LOW); // CE pin - direct addressing below
  PORTB &= ~0x02;
  SPI.transfer(0x02);  // save data from memory in sequence mode
  SPI.transfer((address>>16) & 0xff); // write 3 byte address most significant first 
  SPI.transfer((address>>8) & 0xff);
  SPI.transfer(address & 0xff);
  SPI.transfer(data >> 8);
  SPI.transfer(data & 0xff);
  //digitalWrite(CS1_BAR, HIGH); // CE pin - direct addressing below
  PORTB |= 0x02; 
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

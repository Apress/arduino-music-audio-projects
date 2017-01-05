// Karplus Due - Mike Cook
// code for the Arduino Due
// takes up 3.8uS in interrupt
// every 22.8 uS

const int bufferMax = 550;
volatile int outBuffer[bufferMax];
volatile int bufferSize = 200;
volatile int inPoint = bufferSize -1;
volatile int outPoint1 = 0;
volatile int outPoint2 = 1;
const byte trigger[] = {7,6,5,4,3,2}; // pin to trigger the sound
// 41000 divided by frequency = buffer size required
int bufferToUse [] = {535, 401, 300, 225, 179, 134};
boolean lastPush [6];

void setup() {
  Serial.begin(250000);
  pinMode(13,OUTPUT);
  for(int i =0; i< 6; i++){
     pinMode(trigger[i], INPUT_PULLUP); // strike string 
  }
  analogWriteResolution(12);
  analogWrite(DAC0,2048);
  setupTimer();
}

void setupTimer(){
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk(ID_TC4);    // enable the timer clock 

  /* we want wavesel 01 with RC */
  TC_Configure(/* clock */TC1,/* channel */1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK2);
  TC_SetRC(TC1, 1, 238); // sets to about 44.1 Khz interrupt rate
  TC_Start(TC1, 1);
 
  // enable timer interrupts
  TC1->TC_CHANNEL[1].TC_IER=TC_IER_CPCS;
  TC1->TC_CHANNEL[1].TC_IDR=~TC_IER_CPCS;
 
  /* Enable the interrupt in the nested vector interrupt controller */
  /* TC4_IRQn where 4 is the timer number * timer channels (3) + the channel number (=(1*3)+1) for timer1 channel1 */
  NVIC_EnableIRQ(TC4_IRQn);
}

void loop() {
  boolean push;
  for(int i=0; i<7; i++){
  push = digitalRead(trigger[i]);
  if(push == false && lastPush[i] == true){
    bufferSize = bufferToUse[i];
    primeBuffer(512,8);
    noInterrupts();
    inPoint = bufferSize -1;
    outPoint1 = 0;
    outPoint2 = 1;
    interrupts();
   }
    lastPush[i] = push;
  }
}

void primeBuffer(int energy, int stroke ){
  int primeSample = 0;
  for(int i=0; i<bufferSize/stroke; i++){
    outBuffer[i] = random(energy) - energy >> 1;
    primeSample ++;
   }
   while(primeSample < bufferMax-1) {
    outBuffer[primeSample] = 0;
    primeSample ++;
   }
 }

void TC4_Handler()
{
  digitalWrite(13,HIGH); //  time ISR - flag start
  TC_GetStatus(TC1, 1);  // clear status to allow the timer interrupt to trigger again
   outBuffer[inPoint] = (outBuffer[outPoint1] + outBuffer[outPoint2]) >> 1;
   dacc_write_conversion_data(DACC_INTERFACE, outBuffer[inPoint]+ 2048);
    // move pointers
    inPoint++;
    if(inPoint >= bufferSize -1 ) inPoint = 0;
    outPoint1++;
    if(outPoint1 >= bufferSize -1) outPoint1 = 0;
    outPoint2++;
    if(outPoint2 >= bufferSize -1) outPoint2 = 0; 
  digitalWrite(13,LOW);  //  time ISR - flag end
}


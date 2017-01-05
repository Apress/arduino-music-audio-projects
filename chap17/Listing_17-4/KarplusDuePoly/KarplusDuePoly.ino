// Karplus Due - Mike Cook
// code for the Arduino Due
// takes 11.7uS in interrupt
// every 22.8uS  

const int bufferMax = 550;
volatile int outBuffer[6][bufferMax];
volatile int inPoint [6];
volatile int outPoint1[6];
volatile int outPoint2[6];
// 4100 / frequency = buffer size required
volatile int bufferSize[] = {535, 401, 300, 225, 179, 134};
const byte trigger[] = {7,6,5,4,3,2}; // pin to trigger the sound
boolean lastPush [6];

void setup() {
  pinMode(13,OUTPUT);
  for(int i =0; i< 6; i++){
     pinMode(trigger[i], INPUT_PULLUP); // strike string
     inPoint [i] = bufferSize[i] -1;
     outPoint1[i] = 0; 
     outPoint2[i] = 1;
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
    primeBuffer(512,16,i);
    noInterrupts();
    inPoint[i] = bufferSize[i] -1;
    outPoint1[i] = 0;
    outPoint2[i] = 1;
    interrupts();
   }
    lastPush[i] = push;
  }
}

void primeBuffer(int energy, int stroke, int bufferNo ){
  int primeSample = 0;

  for(int i=0; i<bufferSize[bufferNo]/stroke; i++){
    outBuffer [bufferNo][i] = random(energy) - energy >> 1;
    primeSample ++;
   }
   while(primeSample < bufferMax-1) {
    outBuffer[bufferNo][primeSample] = 0;
    primeSample ++;
   }
 }

void TC4_Handler()
{
  // digitalWrite(13,HIGH); //  time ISR - flag start
  TC_GetStatus(TC1, 1);  // clear status to allow the timer interrupt to trigger again
  int acc = 0;
  for(int b=0; b <6; b++) {  
   outBuffer[b][inPoint[b]] = (outBuffer[b][outPoint1[b]] + outBuffer[b][outPoint2[b]]) >> 1;
   acc += outBuffer[b][inPoint[b]];
    // move pointers
    inPoint[b]++;
    if(inPoint[b] >= bufferSize[b] -1 ) inPoint[b] = 0;
    outPoint1[b]++;
    if(outPoint1[b] >= bufferSize[b] -1) outPoint1[b] = 0;
    outPoint2[b]++;
    if(outPoint2[b] >= bufferSize[b] -1) outPoint2[b] = 0;
  } 
  dacc_write_conversion_data(DACC_INTERFACE, acc + 2048);
  // digitalWrite(13,LOW);  //  time ISR - flag end
}


// Audio Exciter Due - Mike Cook
// code for the Arduino Due
// sample every 22.8uS  44.1KHz  

volatile int16_t transferFunction[4096];
const int nPots = 8;
float potPram[nPots];
int pot[nPots], lastPot[nPots];
int potPin[nPots] = {0, 1, 2, 3, 4, 5, 6, 7};
int samplePin = 10; // input to take the audio from
volatile boolean taken = true;
volatile int sample = 0;
const int triggerPin = 2;
const int timePin = 13;

void setup() {
  Serial.begin(250000);
  REG_ADC_MR = (REG_ADC_MR & 0xFFF0FFFF) | 0x00020000; // master A/D clock
  adc_init(ADC, SystemCoreClock, ADC_FREQ_MAX, ADC_STARTUP_FAST);
  pinMode(timePin,OUTPUT);
  pinMode(triggerPin,INPUT_PULLUP); // trigger new lookup
  analogWriteResolution(12);
  analogReadResolution(12);
  analogWrite(DAC0,2048);
  getPots();
  displayTransfer(); // remove if not displaying plot
  setupTimer(); // go
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

void TC4_Handler() // measured at 3.2uS
{ 
  //digitalWrite(timePin,HIGH); //  time ISR - flag start
  TC_GetStatus(TC1, 1);  // clear status to allow the timer interrupt to trigger again
  sample = transferFunction[sample];
  dacc_write_conversion_data(DACC_INTERFACE, sample);
  taken = true;
  //digitalWrite(timePin,LOW);  //  time ISR - flag end
}

void loop() {
 if(taken){
  sample = analogRead(samplePin);
  taken = false;
 }
 if(!digitalRead(triggerPin)){
  noInterrupts();
  getPots();
  interrupts();
  displayTransfer(); // remove for faster update
 }
}

void getPots(){
      float sum = 0.0;
  for(int i=0; i<nPots; i++){
    pot[i]=analogRead(potPin[i]);
  }
 // work out pot paramaters
    for(int i=0; i<nPots; i++){
       sum += (float)(pot[i]) / 1023.0;
    }
    for(int i=0; i<nPots; i++){
       potPram[i] = ((float)(pot[i]) / 1023.0) / sum ;
    }
    makeLookup(); // create new lookup table
 } 

void makeLookup(){
  float res = 2.0 / 4096.0 ;
  float n = -1;
  float value = 0;
  for(int i = 0; i<4096 ; i++){
    value = potPram[0]*fundmental(n) + potPram[1]*secondH(n) + potPram[2]*thirdH(n) +
            potPram[3]*forthH(n) + potPram[4]*fifthH(n) + potPram[5]*sixthH(n) +
            potPram[6]*seventhH(n) + potPram[7]*eighthH(n);
    transferFunction[i] = (int16_t)(value*2047 + 2047);
    n += res;
  }
}

float fundmental(float n){
  return n;
}

float secondH(float n){
    n = 2*pow(n,2.0) - 1;
  return n;
}

float thirdH(float n){
    n = 4*pow(n,3.0) - 3*n;
  return n;
}

float forthH(float n){
    n = 8*pow(n,4.0) - 8*pow(n,2.0) + 1;
  return n;
}

float fifthH(float n){
    n = 16*pow(n,5.0) - 20*pow(n,3.0) + 5*n;
  return n;
}

float sixthH(float n){
    n = 32*pow(n,6.0) - 48*pow(n,4.0) + 18*pow(n,2.0) - 1;
  return n;
}

float seventhH(float n){
    n = 64*pow(n,7.0) - 112*pow(n,5.0) + 56*pow(n,3.0) - 7*n;
  return n;
}
float eighthH(float n){
    n = 128*pow(n,8.0) - 256*pow(n,6.0) + 160*pow(n,4.0) - 32*pow(n,2.0) + 1;
  return n;
}

void displayTransfer(){
  for(int i=0; i<500; i++){
    Serial.println(transferFunction[i*8]);
  }
  for(int i=0; i<96; i++){
    Serial.println(0);
  }
}
  



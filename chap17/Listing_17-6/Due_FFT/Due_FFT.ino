/*
 * Light spectrum - FFT by Mike Cook
 * for Arduino Due & Neopixel strip
 * Each bin covers 23.4375 Hz
 */
#include <SplitRadixRealP.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN   3 // pin connected to the NeoPixels
#define NUMPIXELS   240 // number of LEDs 240 LEDs in 4 meters
#define xMax 20 // maximum number of coloumns
#define yMax 12 // maximum number of rows

#define   SMP_RATE          48000UL 
#define   CLK_MAIN       84000000UL
#define   TMR_CNTR       CLK_MAIN / (2 *SMP_RATE)

// FFT_SIZE IS DEFINED in Header file Radix4.h 
// #define   FFT_SIZE  2048

#define   MIRROR   FFT_SIZE / 2  // mirror image of bins
#define   INP_BUFF FFT_SIZE      // input buffer for samples
         
volatile   uint16_t   sptr = 0 ;
volatile    int16_t   flag = 0 ;
 uint16_t  inp[2][INP_BUFF]   = { 0};     // DMA likes ping-pongs buffer
       int    f_r[FFT_SIZE]   = { 0};
       int    out1[MIRROR]    = { 0};     // Magnitudes
       int    out2[MIRROR]    = { 0};     // Magnitudes
const  int    dc_offset       = 2047;    // vertual ground 
              
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
SplitRadixRealP     radix;

//const int binSplit[] = {1,4,9,13,17,22,26,30,35,38,43,85,128,171,213,256,299,341,384,469,512};
// Alternative bin split
const int binSplit[] ={1,4,5,7,9,12,15,20,25,32,43,55,71,92,119,154,196,252,324,427,555,713};
int binTotal[20];
int angleInc = 360 / (xMax / 2);
int partAngleInc = angleInc / yMax;
// cosmetic value to scale bins by - set to taste
int scale[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; 

void pio_TIOA0 () { // Configure Ard pin 2 as output from TC0 channel A (copy of trigger event)
  PIOB->PIO_PDR = PIO_PB25B_TIOA0 ;  // disable PIO control
  PIOB->PIO_IDR = PIO_PB25B_TIOA0 ;   // disable PIO interrupts
  PIOB->PIO_ABSR |= PIO_PB25B_TIOA0 ;  // switch to B peripheral
}
 
void timer_setup (){
  pmc_enable_periph_clk(TC_INTERFACE_ID + 0 *3 + 0); // clock the TC0 channel 0
  TcChannel * t = &(TC0->TC_CHANNEL)[0] ;            // pointer to TC0 registers for its channel 0
  t->TC_CCR = TC_CCR_CLKDIS ;                        // disable internal clocking while setup regs
  t->TC_IDR = 0xFFFFFFFF ;                           // disable interrupts
  t->TC_SR ;                                         // read int status reg to clear pending
  t->TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1 |           // use TCLK1 (prescale by 2, = 42MHz)
              TC_CMR_WAVE |                          // waveform mode
              TC_CMR_WAVSEL_UP_RC |                  // count-up PWM using RC as threshold
              TC_CMR_EEVT_XC0 |     // Set external events from XC0 (this setup TIOB as output)
              TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_CLEAR |
              TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_CLEAR ;
  
  t->TC_RC = TMR_CNTR;              // counter resets on RC, so sets period in terms of 42MHz clock
  t->TC_RA = TMR_CNTR /2;           // roughly square wave
  t->TC_CMR = (t->TC_CMR & 0xFFF0FFFF) | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET ;  // set clear and set from RA and RC compares
  t->TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG ;  // re-enable local clocking and switch to hardware trigger source.  
}

void adc_setup (){
  pmc_enable_periph_clk(ID_ADC);
  adc_init(ADC, SystemCoreClock, ADC_FREQ_MAX, ADC_STARTUP_FAST);
  NVIC_EnableIRQ (ADC_IRQn);               // enable ADC interrupt vector
  adc_disable_all_channel(ADC);
  adc_enable_interrupt(ADC, ADC_IER_RXBUFF);
  ADC->ADC_RPR  =  (uint32_t)  inp[0];      // DMA buffer
  ADC->ADC_RCR  =  INP_BUFF;
  ADC->ADC_RNPR =  (uint32_t)  inp[1];      // next DMA buffer
  ADC->ADC_RNCR =  INP_BUFF;
  ADC->ADC_PTCR =  1;
  adc_set_bias_current(ADC, 0x01); 
  adc_enable_channel(ADC, ADC_CHANNEL_7);  // AN0
  adc_configure_trigger(ADC, ADC_TRIG_TIO_CH_0, 0);
  adc_start(ADC); 
}

void ADC_Handler (void){
 if((adc_get_status(ADC) & ADC_ISR_RXBUFF) ==  ADC_ISR_RXBUFF){
    flag = ++sptr; 
    sptr &=  0x01; // alternate buffer to use
    ADC->ADC_RNPR  =  (uint32_t)  inp[sptr];
    ADC->ADC_RNCR  =  INP_BUFF;
    }
}

void setup(){
  Serial.begin (250000); // fast baud
  pixels.begin(); // This initialises the NeoPixel library
  wipe();
  pixels.show();
  adc_setup ();         
  timer_setup ();         
  pinMode( 2, INPUT); //
 }

inline int mult_shft12( int a, int b){
  return (( a  *  b )  >> 12);      
}

void loop(){
  if (flag){   
   uint16_t indx_a = flag -1;
   uint16_t indx_b = 0;  
   for ( uint16_t i = 0, k = (NWAVE / FFT_SIZE); i < FFT_SIZE; i++ ){  
      uint16_t windw = Hamming[i * k];
      f_r[i] = mult_shft12((inp[indx_a][indx_b++] - dc_offset), windw);
   }
   radix.rev_bin( f_r, FFT_SIZE);
   radix.fft_split_radix_real( f_r, LOG2_FFT);
   radix.gain_Reset( f_r, LOG2_FFT -1); 
   radix.get_Magnit1( f_r, out1);
   radix.get_Magnit2( f_r, out2);
    // Magnitudes:- 
   bufferToBinTotal();
   //showBins(); // show in plot window 
   binsToLEDs();  
   // delay(200);
   flag = 0;
   }   
}

void bufferToBinTotal(){
  int number = 0;
    for(int i=0; i< 20; i++){
      number = 0;
       binTotal[i] = 0;
    for(int k = binSplit[i]; k < binSplit[i+1]; k++){
      binTotal[i] += out1[k];
      number++;
    }
    binTotal[i] = binTotal[i] / number;
  }
}

void showBins(){ // to plot window
  int bin = 0;
  for(int i=0; i< 20; i++){
    for(int k=0; k < 15; k++){
      Serial.println(binTotal[i]);
    }
    for(int k=0; k < 10; k++){
      Serial.println(0);
    }
  }
}

void binsToLEDs(){
  wipe();
  for(int i=0; i< 20; i++){
   setStrip(i, binTotal[i] /scale[i]);
 }
   pixels.show();
}

void setStrip(int strip, int height){
  int stripColour;
  int angle = angleInc * strip;
  if(height > yMax) height = yMax;
  for(int y=0; y < height; y++){
      stripColour = colorH(angle + (y * partAngleInc) );
      pixels.setPixelColor(getPixPos(strip,y) , stripColour);
  }
}
  
int getPixPos(int x, int y){ // for a serpentine raster
   int pos;
   if(x &0x1) {
      pos = x * yMax + (yMax -1 - y) ;
   } else {
   pos = x * yMax + y;
   } 
   return pos;
}

inline int colour( int a, int b, int c){
  return ((a << 16) | (b << 8) | c);      
}

int colorH(int angle){ //# color returned H=angle, S=1, V=1
    // # get angle in range 0 to 255
    while (angle <0 ) angle += 256;   
    while (angle > 255) angle -=256;          
    if (angle < 85) return colour(255 - angle*3, angle*3, 0);        
    if (angle < 170){
        angle -= 85;
        return colour(0, 255 - angle * 3, angle * 3);
    }
    angle -= 170;
    return colour(angle * 3, 0, 255 - angle * 3);
}

void wipe(){
    for(int i=0;i<NUMPIXELS;i++){
       pixels.setPixelColor(i, pixels.Color(0,0,0)); 
       }
}


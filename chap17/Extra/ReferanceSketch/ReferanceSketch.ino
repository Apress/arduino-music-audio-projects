// Direct wave from two tables - hacked by Mike Cook
/* writing to either DAC
 * --- Code: ---  /* write to DAC0 */
  dacc_set_channel_selection(DACC_INTERFACE, 0);
  dacc_write_conversion_data(DACC_INTERFACE, some_value);
  /* write to DAC1 */
  dacc_set_channel_selection(DACC_INTERFACE, 1);
  dacc_write_conversion_data(DACC_INTERFACE, some_other_value);

 * /
// RCArduino DDS Sinewave for Arduino Due
// RCArduino DDS Sinewave by RCArduino is licensed under a Creative Commons Attribution 3.0 Unported License.
// Based on a work at rcarduino.blogspot.com.

// For helpful background information on Arduino Due Timer Configuration, refer to the following link
// thanks to Sebastian Vik
// http://arduino.cc/forum/index.php?action=post;topic=130423.15;num_replies=20

// For background information on the DDS Technique see
// http://interface.khm.de/index.php/lab/experiments/arduino-dds-sinewave-generator/

// For audio sketches making extensive use of DDS Techniques, search the RCArduino Blog
// for the tags Audio or synth

// These are the clock frequencies available to the timers /2,/8,/32,/128
// 84Mhz/2 = 42.000 MHz
// 84Mhz/8 = 10.500 MHz
// 84Mhz/32 = 2.625 MHz
// 84Mhz/128 = 656.250 KHz
//
// 44.1Khz = CD Sample Rate
// Lets aim for as close to the CD Sample Rate as we can get -
//
// 42Mhz/44.1Khz = 952.38
// 10.5Mhz/44.1Khz = 238.09 // best fit divide by 8 = TIMER_CLOCK2 and 238 ticks per sample
// 2.625Hmz/44.1Khz = 59.5
// 656Khz/44.1Khz = 14.88

// 84Mhz/44.1Khz = 1904 instructions per tick

// the phase accumulator points to the current sample in our wavetable
uint32_t ulPhaseAccumulator = 0;
// the phase increment controls the rate at which we move through the wave table entries
// higher values = higher frequencies
volatile uint32_t ulPhaseIncrement = 0;   // 32 bit phase increment, see below
uint32_t ulTableAccumulator = 0;
// the table increment controls the rate at which we move between the two wave tables
// higher values = higher transition speed
volatile uint32_t ulTableIncrement = 0x4000;   // table increment speed


// full waveform = 0 to SAMPLES_PER_CYCLE
// Phase Increment for 1 Hz =(SAMPLES_PER_CYCLE_FIXEDPOINT/SAMPLE_RATE) = 1Hz
// Phase Increment for frequency F = (SAMPLES_PER_CYCLE/SAMPLE_RATE)*F
#define SAMPLE_RATE 44100.0
#define SAMPLES_PER_CYCLE 600
#define SAMPLES_PER_CYCLE_FIXEDPOINT (SAMPLES_PER_CYCLE<<20)
#define TICKS_PER_CYCLE (float)((float)SAMPLES_PER_CYCLE_FIXEDPOINT/(float)SAMPLE_RATE)

// to represent 600 we need 10 bits
// Our fixed point format will be 10P22 = 32 bits


// We have 521K flash and 96K ram to play with

// Create a table to hold the phase increments we need to generate midi note frequencies at our 44.1Khz sample rate
#define MIDI_NOTES 128
uint32_t nMidiPhaseIncrement[MIDI_NOTES];

// fill the note table with the phase increment values we require to generate the note
void createNoteTable(float fSampleRate)
{
   float fFrequency;
  for(uint32_t unMidiNote = 0;unMidiNote < MIDI_NOTES;unMidiNote++)
  {
    // Correct calculation for frequency
   // Serial.print(unMidiNote);
    // Serial.print(" ");
     fFrequency = ((pow(2.0,(unMidiNote-69.0)/12.0)) * 440.0);
    // Serial.print(fFrequency);
    // Serial.print(" ");
   
    nMidiPhaseIncrement[unMidiNote] = fFrequency*TICKS_PER_CYCLE;
    // Serial.println(nMidiPhaseIncrement[unMidiNote]);
  }
}

// Create a table to hold pre computed sinewave, the table has a resolution of 600 samples
#define WAVE_SAMPLES 600
// default int is 32 bit, in most cases its best to use uint32_t but for large arrays its better to use smaller
// data types if possible, here we are storing 12 bit samples in 16 bit ints
uint16_t nSineTable[2][WAVE_SAMPLES];

// create the individual samples for our sinewave table
void createSineTable()
{
  for(uint32_t nIndex = 0;nIndex < WAVE_SAMPLES;nIndex++)
  {
    // sin wave normalised to 12 bit range 0-2048
    nSineTable[0][nIndex] = (uint16_t)  (((1+sin(((2.0*PI)/WAVE_SAMPLES)*nIndex))*4095.0)/2);
    // saw wave  normalised to 12 bit range 0-2048
    nSineTable[1][nIndex] = (uint16_t) (nIndex * (4095.0 / (float)WAVE_SAMPLES) );
    // Serial.println(nSineTable[nIndex]);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(13,OUTPUT); // flag to time ISR
//  pinMode(12,OUTPUT);
  createNoteTable(SAMPLE_RATE);
  createSineTable();
 
  /* turn on the timer clock in the power management controller */
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk(ID_TC4);

  /* we want wavesel 01 with RC */
  TC_Configure(/* clock */TC1,/* channel */1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK2);
  TC_SetRC(TC1, 1, 238); // sets <> 44.1 Khz interrupt rate
  TC_Start(TC1, 1);
 
  // enable timer interrupts on the timer
  TC1->TC_CHANNEL[1].TC_IER=TC_IER_CPCS;
  TC1->TC_CHANNEL[1].TC_IDR=~TC_IER_CPCS;
 
  /* Enable the interrupt in the nested vector interrupt controller */
  /* TC4_IRQn where 4 is the timer number * timer channels (3) + the channel number (=(1*3)+1) for timer1 channel1 */
  NVIC_EnableIRQ(TC4_IRQn);

  // this is a cheat - enable the DAC
  analogWrite(DAC0,0);
}

void loop()
{
  // read analog input 0 drop the range from 0-1024 to 0-127 with a right shift 3 places,
  // then look up the phaseIncrement required to generate the note in our nMidiPhaseIncrement table
  uint32_t ulInput = analogRead(0);
  ulPhaseIncrement = nMidiPhaseIncrement[ulInput>>3];
}

void TC4_Handler()
{
    digitalWrite(13,HIGH); //  time ISR - flag start
//    digitalWrite(12,HIGH);
  // We need to get the status to clear it and allow the interrupt to fire again
  TC_GetStatus(TC1, 1);
 
  ulPhaseAccumulator += ulPhaseIncrement;   // 32 bit phase increment, see below
  ulTableAccumulator += ulTableIncrement;   // 32 bit table increment,

  // if the phase accumulator over flows - we have been through one cycle at the current pitch,
  // now we need to reset the grains ready for our next cycle
  if(ulPhaseAccumulator > SAMPLES_PER_CYCLE_FIXEDPOINT)
  {
   ulPhaseAccumulator = 0;
  }
  // if the table accumulator over flows - we have been through one cycle at the current pitch,
  // now we need to reset the grains ready for our next cycle
  if(ulTableAccumulator >= 0x1000000)
  {
   ulTableAccumulator = 0;
  }

  // get the current sample  
  // float part = (float(ulTableAccumulator>>20)/10.0);
  uint32_t ulOutput1 = nSineTable[0][ulPhaseAccumulator>>20];
  uint32_t ulOutput2 = nSineTable[1][ulPhaseAccumulator>>20];
  uint32_t ulOutput = (((ulTableAccumulator>>20)/16.0) * ulOutput1) + (((1-(ulTableAccumulator>>20)/16.0)) * ulOutput2 ) ;
 
  // we cheated and user analogWrite to enable the dac, but here we want to be fast so
  // write directly 
  dacc_write_conversion_data(DACC_INTERFACE, ulOutput);
  digitalWrite(13,LOW);  //  time ISR - flag end
//  digitalWrite(12,LOW);
}

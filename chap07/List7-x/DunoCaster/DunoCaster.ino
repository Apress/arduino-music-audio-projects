
/* DunoCaster Midi Guitar - Mike Cook
*/
// Libraries to include
#include <Wire.h>
#include <Arduino.h>
#include "Chords.h"
#include "Defines.h"
#include "Variables.h"

// Start of code
void setup() {
  //  set the states of the Arduino I/O pins:
   pinMode(chordChange, INPUT_PULLUP);
   pinMode(stringChange, INPUT_PULLUP); 
   pinMode(controlChange, INPUT_PULLUP);
   pinMode(thumbIn, INPUT_PULLUP);       // Thumb  first
   pinMode(R1D, INPUT);                // Rotary switch 1 direction
   pinMode(R2D, INPUT);                // Rotary switch 2 direction
   pinMode(chordShowLEDr, OUTPUT);            // Tri colour LED Red
   pinMode(chordShowLEDg, OUTPUT);            // Tri colour LED Green
   pinMode(chordShowLEDb, OUTPUT);            // Tri colour LED Blue
   analogWrite(chordShowLEDr, 255);            // Tri colour LED Red
   analogWrite(chordShowLEDg, 255);            // Tri colour LED Green
   analogWrite(chordShowLEDb, 255);            // Tri colour LED Blue
   
 // Setup I2C devices
   Wire.begin();                      // start the I2C interface
   // Initialise port expanders INT internal connected + open drain int
   gpio_write(chordSwitchAddress, (MIRROR | ODR)<<8, IOCON);
   gpio_write(stringSwitchAddress, (MIRROR | ODR)<<8, IOCON); 
   gpio_write(controlSwitchaddress, (MIRROR | ODR)<<8, IOCON);
   gpio_write(chordLEDaddress, (MIRROR | ODR)<<8, IOCON);
         
   gpio_write(chordLEDaddress, 0x0, IODIR);             // set to all output
   gpio_write(stringSwitchAddress, ddrString, IODIR);      // String triggers
   gpio_write(stringSwitchAddress, ddrString, GPINTEN);   // enable interrupt on change 
  
   gpio_write(controlSwitchaddress, ddrControl, IODIR);    // Control Switches
   gpio_write(controlSwitchaddress, ddrControl, IPOL);       // invert switch inputs so 1 = pressed
   gpio_write(controlSwitchaddress, ddrControl, GPINTEN);   // enable interrupt on change
   gpio_write(controlSwitchaddress, ddrControl & 0xfffC, GPPU);   // enable pullups on all inputs but lower two
   
   gpio_write(chordSwitchAddress, ddrChord, IODIR);     // top 6 bits output rest to all inputs
   gpio_write(chordSwitchAddress, ddrChord, IPOL);     // invert switch inputs so 1 = pressed
   gpio_write(chordSwitchAddress, ddrChord, GPINTEN);   // enable interrupt on change
   gpio_write(chordSwitchAddress, ddrChord, GPPU);   // enable pullups on inputs 
   
// set up initial state of the LEDs
   gpio_write(chordLEDaddress, redChordLED, OLAT);      // turn off all LEDs
   gpio_write(stringSwitchAddress, greenChordLED, OLAT);   // turn off all LEDs
   gpio_write(chordSwitchAddress, stringLED, OLAT);        // turn off all LEDs
   gpio_write(controlSwitchaddress, modeLED, OLAT);        // Initilise Mode red LEDs
   
   lastString = gpio_read(stringSwitchAddress);   // get initial state of switches
   lastControl = gpio_read(controlSwitchaddress);

 //  Setup serial / MIDI
 //  Serial.begin(31250);       // MIDI speed
   Serial.begin(9600);    // Debug speed
   programChange(0xc0, 25);  // Change MIDI voice to guitar
  
   pickTime = millis() + rot2;  // initial basic tempo of notes
   doControl();  // set up options on switch
   lastControl = 0xffff;  // do it twice to make sure software thinks all inputs have change
   doControl();
}

//********************* MAIN LOOP ***********************************

void loop() {
  if(playing) {
   // check for updating sound
   if(millis() > pickTime){    // time to update notes or tempo LED
    if(patPlaying) { 
       if(delCount==0) doNextPick(); else delCount--;       
     }
     else {pickTime = millis() + rot2; }    // set the next time to look at the notes
     metronome++; // Count the periods
     if(metronome > 7){  // every 8th tick toggle the light
       if((modeLED & pickingSpeedLEDr) == 0) modeLED ^= pickingSpeedLEDg;
        modeLED ^= pickingSpeedLEDr;
        gpio_write(controlSwitchaddress, modeLED , OLAT);
        metronome = 0;
      //  Serial.print(" time "); Serial.println(rot2, HEX);
     }
   }  
   // check for chord change
     if(digitalRead(chordChange) == LOW){changeChord();}
     
    // check for touch sensors
    if(digitalRead(stringChange) == LOW){doString();}
    
   // check for thumb stimlus
    if(digitalRead(thumbIn) == LOW && patPlaying == false){ patPlaying = true;} 
     
    // check for change to control switches
    if(digitalRead(controlChange) == LOW){doControl();}
  }
  else {  // if not playing then in setup loop 
           // check for change to control switches
       if(digitalRead(controlChange) == LOW){
       doControl();
       doCapo();
       doChangeSound();
       }
      }
    } // end loop function
    
//********************* Functions *********************************** 
void doControl(){
    int control = gpio_read(controlSwitchaddress);    // get current state of inputs
    int change = control ^ lastControl;  // find what had changed
 //   Serial.print(control,HEX); Serial.print("  "); Serial.println(change, HEX);
  // Check and adjust rotary inputs
   if((change & R1S) != 0) {   
              if(digitalRead(R1D) == LOW) rot1++; else {
                if(rot1 > 0) rot1--; } 
                   if(rot1 > 127) rot1=127;
                  //   Serial.print(rot1, DEC);  Serial.print(" / ");  Serial.println(rot2, DEC);  
              }  // update speed (later change to mode dependent update)
    if((change & R2S) != 0) {     // a rotary switch has changed see which one, and adjust count
              if(digitalRead(R2D) == LOW) rot2++; else {
                if(rot2 > 0) rot2--; }
                ; 
                 //  Serial.print(rot1, DEC);  Serial.print(" / ");  Serial.println(rot2, DEC); 
              }

   // Check and update switch inputs
    if( (change & pushB) !=0 ){
     //  Serial.println("push");
     if((control & pushB) == 0) push = true;
    }
    
    if( (change & playSwitch) !=0 ){
     //  Serial.println("switch 1");
     if((control & playSwitch) == 0) { 
       playing = false;
       gpio_write(chordSwitchAddress, 0xfc00 , OLAT);  // turn on green LEDs
       stringLED &= ~auxLEDg;
       stringLED &= ~0xf03f;  // remove red LEDs
       stopNotes();
       rot2PlayStore = rot2;  // save and restore rotary 2 values
       rot2= rot2StoreVoice;
       gpio_write(stringSwitchAddress, (rot2 << 6) & 0x0fc0 , OLAT); // display count
      //    Serial.print(" into voice setup "); Serial.println(rot2, HEX);
         }
       else {
         playing = true;
        gpio_write(chordSwitchAddress, 0x0000 , OLAT); // turn off green LEDs
        stringLED |= auxLEDg;
        stringLED &= 0xf03f;  // remove red LEDs
               gpio_write(stringSwitchAddress, stringLED , OLAT);               
               rot2StoreVoice =  rot2; // save and restore rotory 2 values
               rot2 = rot2PlayStore;
     //          Serial.print(" into play mode "); Serial.println(rot2, HEX);
          }

    } 

     // Serial.println("Aux switch");
     if((control & auxSwitch) == 0) modeLED &= ~auxLEDr; else modeLED |= auxLEDr;
     if( ((change & octave1Switch) !=0 ) || ((change & octave2Switch) !=0 ) ){
        if((control & octave1Switch) == 0) {
        modeLED &= ~capoLEDr; 
        octave1SwitchState =true;  
      } 
    else {
     modeLED |= capoLEDr; 
     octave1SwitchState =false;
    }
     if((control & octave2Switch) == 0) {
       modeLED &= ~capoLEDg; 
       octave2SwitchState =true; 
     } 
   else { 
     modeLED |= capoLEDg; 
     octave2SwitchState =false; 
   }
      gpio_write(controlSwitchaddress, modeLED , OLAT);
      patShift=4;
      if(octave1SwitchState) patShift = 0;
      if(octave2SwitchState) patShift = 8;
     }
     
     if( ((change & pick2Switch) !=0 ) || ((change & pick1Switch) !=0 ) ){ 
       if((control & pick2Switch) == 0) { 
          stringLED &= ~pickingLEDg;  
          pick1SwitchState =true; 
      } 
    else {
      stringLED |= pickingLEDg; 
      pick1SwitchState =false;
   } 
     if((control & pick1Switch) == 0) {
         modeLED &= ~pickingLEDr; 
         pick2SwitchState = true;
       } 
     else {
       modeLED |= pickingLEDr; 
       pick2SwitchState = false;
     }        
        gpio_write(stringSwitchAddress, stringLED , OLAT);
        gpio_write(controlSwitchaddress, modeLED , OLAT);
        octave = 12;  // its faster than multiplying
        if(pick2SwitchState) octave = 0;
        if(pick1SwitchState) octave = 24;
 //       Serial.print(" octave value is ");  Serial.println(octave, DEC);  
     }
    lastControl = control;  // save state to look for change next time 
}

void doString(){ 
       int string, stringState;
       string = gpio_read(stringSwitchAddress);
    //   Serial.print(string, HEX); Serial.print(" - ");
       stringState = string ^ lastString;
       if( ( stringState & finger1) !=0 ) {
                         if(((string & finger1) == 0) && (patPlaying == false)){ 
                           patPlaying = true;
                           pat=patShift;  // pattern to play
                           }
                         }
       if( ( stringState & finger2) !=0 ) { 
                         if(((string & finger2) == 0) && (patPlaying == false)){ 
                           patPlaying = true;
                           pat=1 + patShift;  // pattern to play
                           }
                         }
       if( ( stringState & finger3) !=0 ) {
                         if(((string & finger3) == 0) && (patPlaying == false)){ 
                           patPlaying = true;
                           pat=2 + patShift;  // pattern to play
                           } 
                         }
       if( ( stringState & finger4) !=0 ) {
                        if(((string & finger4) == 0) && (patPlaying == false)){ 
                           patPlaying = true;
                           pat=3 + patShift;  // pattern to play
                           } 
                         }
      lastString = string;
}

void doNextPick(){  // output next note in picking pattern
       do{
        if( pattern[pat][pickNum] != 6){  // only play a note if it is not the last note in the sequence
         int lastString = playString;
          playString = pattern[pat][pickNum];     // get the string to play
          if(note[playString] !=0) {        // is there a note to play in this chord
            if(notePlaying[playString]) {   // if string is sounding then turn it off
            noteSend(0x80, notePlaying[playString], 0x00);
            }
            if(note[playString] !=0){
            notePlaying[playString] = note[playString];
            noteSend(0x90, note[playString], rot1);
            if(lastDel !=0 ) stringLED &= 0xf003 ; // turn off other string LEDs unless last delay was zero
            stringLED |= 1 << (playString + 6);     // turn on this string LED
            gpio_write(stringSwitchAddress, stringLED , OLAT);
            }
            delCount = lastDel = patternDelay[pat][pickNum];       // delay count
          } 
            pickTime = millis() + rot2;  }   // set time for next pick if there is something else in the pattern
          
             else {  // end of sequnce
            patPlaying = false;
            delCount = 0;
            lastDel = 1;   // so LED is turned off on next pick
            pickNum = -1;   // increment will take it to zero
            // do not unpdate pickTime so that the end of the start of the next pattern immediatly follows the last pattern
          }
          pickNum++;
       } while(patternDelay[pat][pickNum] == 0);
}

void doCapo(){
  int bar;
  if(rot2 > 6) rot2=6;
  if(push){
       capo = rot2; 
       bar = 0x0;
     }
     else bar = (0x3f << rot2) & 0x0fc0;
  gpio_write(stringSwitchAddress, bar , OLAT);
}

void doChangeSound(){
  if(rot2 > 63) rot2=63;
  if(push){
 //      Serial.print(" sound number ");  Serial.print(pgm_read_byte_near(&soundBank[rot2]), DEC);
 //      Serial.print(" bank number ");  Serial.println(pgm_read_byte_near(&bankLSB[rot2]), DEC);
         bankChange(0,0);
         bankChange(32, pgm_read_byte_near(&bankLSB[rot2]));  // Change the bank LSB
         programChange(0xc0, pgm_read_byte_near(&soundBank[rot2]));  // Change MIDI voice
         gpio_write(chordSwitchAddress, 0x0000 , OLAT);  // turn off green LEDs         
         while((gpio_read(controlSwitchaddress) & pushB) ==0){ delay(10) ;} // do nothing while still held down
         gpio_write(chordSwitchAddress, 0xfc00 , OLAT);  // turn on green LEDs
   //      gpio_write(stringSwitchAddress, (rot2 << 6) & 0x0fc0 , OLAT); // display count
       push = false;
     }
     else { 
          gpio_write(stringSwitchAddress, (rot2 << 6) & 0x0fc0 , OLAT); // display count
     }
  }

//  plays a MIDI note
void noteSend(byte cmd, byte data1, byte data2) {
  cmd = cmd | midiChannel;  // merge channel number
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}
//  change the voice
void programChange(byte cmd, byte data1) {
  cmd = cmd | midiChannel;  // merge channel number
  Serial.write(cmd);
  Serial.write(data1);
}
//  change the bank
void bankChange(byte cmd, byte data1) {
  Serial.write(0xB0 | midiChannel);  // control change
  Serial.write(cmd);
  Serial.write(data1);
}

// Output Red / Green LEDs
void LEDwrite(int r, int g) {
  int i;
  i = r | (g << 10);
   gpio_write(chordLEDaddress, i , OLAT);
    stringLED &= 0x0fc3;  // leave the string LEDs alone
    stringLED |= (g << 6) & 0xf003;  /// add in the green chord LEDs
  gpio_write(stringSwitchAddress, stringLED , OLAT);
}


// stop the notes playing on all strings
void stopNotes() {
  int i;
   for(i=0;i<6;i++){
  if (notePlaying[i] != 0) {
      noteSend(0x80, notePlaying[i], 0x00);
     notePlaying[i] = 0;
   }
 }
  stringLED &= 0xf003;  // red string LEDs off
  gpio_write(stringSwitchAddress, stringLED, OLAT);
}

// we need to change the chord
void changeChord() {
  int newState =0;
  int mask;
  int chord = 1;

   newState  = gpio_read(chordSwitchAddress);    // get bit pattern of chord switches
   if((newState & 0x7f) != 0){
   redChordLED = 1;     // set top chord LED
   greenChordLED = 0xE; // plus light the three after it
   mask = 0xf;
   while( (newState & redChordLED) == 0) {  // shift the LED pattern until it matches the switch
     redChordLED = redChordLED << 1;
     greenChordLED = greenChordLED << 1;
     mask = mask << 1;
     chord++;   // move to next chord
   }
    rgbSet(chordColourR[chord],chordColourG[chord],chordColourB[chord]);
     LEDwrite(newState & mask, greenChordLED);
     // add any modifier keys to base chord
         // tidy up when all chords are drefined
     redChordLED = redChordLED << 1;
     if( (redChordLED & newState ) !=0 ) chord += 7;
          redChordLED = redChordLED << 1;
     if( (redChordLED & newState ) !=0 ) chord += 14;
  }
      else { 
      LEDwrite(0, 0);  // if nothing held down turn LEDs off and all notes
      rgbSet(0,0,0); // turn chord colour LED off
      chord = 0;
      stopNotes();
           }
  // copy bank into current notes
     for(int j=0;j<6;j++){
     note[j]=pgm_read_byte_near(&chordBank[chord][j]);
     if(note[j] != 0) note[j] += octave + capo;
 //    Serial.print(octave,DEC); Serial.print(" note number ");  Serial.println(note[j], DEC);
       }        
}
void rgbSet(int r, int g, int b){
     analogWrite(chordShowLEDr, 255-r);
     analogWrite(chordShowLEDg, 255-g);
     analogWrite(chordShowLEDb, 255-b);
     }

void gpio_write(int address, int data, int reg) {
  //  Send output register address
  Wire.beginTransmission(address);
  Wire.write(reg);
  //  Connect to device and send two bytes
  Wire.write(0xff & data);  //  low byte
  Wire.write(data >> 8);    //  high byte
  Wire.endTransmission();
}

int gpio_read(int address) {
  int data = 0;
 //  Send input register address
  Wire.beginTransmission(address);
  Wire.write((byte)GPIO);
  Wire.endTransmission();
  //  Connect to device and request two bytes
  Wire.requestFrom(address, 2);
 if (!Wire.available()) { } // do nothing until data arrives
    data = Wire.read();

 if (!Wire.available()) { } // do nothing until data arrives
    data |= Wire.read() << 8;
  Wire.endTransmission();
  return data;
}


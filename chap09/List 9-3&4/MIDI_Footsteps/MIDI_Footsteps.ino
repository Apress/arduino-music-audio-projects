
/* Midi Footsteps - Mike Cook Feb 2014
 *
 * ----------------- 
 * send MIDI serial data, for multiplexed pressure pad sensors
 * 
*/
#include <Wire.h>
#include "Defines.h"

char control[16] = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
// key of C
char notes[16] = { 48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74 };

void setup() {
  //  set the states of the I/O pins to drive the sensor multiplexer:
   pinMode(s0Pin, OUTPUT);
   pinMode(s1Pin, OUTPUT);
   pinMode(s2Pin, OUTPUT);
   pinMode(notesSelect, INPUT_PULLUP);
   lastLedVal = 0;

 // Setup I2C devices
   Wire.begin();                      // start the I2C interface
    gpio_write(ledAddress, (MIRROR | ODR)<<8, IOCON);
   // Initilise registers 
   gpio_write(ledAddress, ddrTrigger, IODIR);   // Make into outputs
   gpio_write(ledAddress, 0, OLAT);            // turn them all off
   doSensorScan();                            // get initial states
   saveCurrentState();
   Serial.begin(31250);       // MIDI speed
}

void loop() {
    doSensorScan();
    lookForChange();
    saveCurrentState();
    } // end loop function
     

void doSensorScan() {  // look at all the sensors
 for(int i=0; i<8; i++){
   // select multiplexer channel
   digitalWrite(s0Pin, i & 0x1);
   digitalWrite(s1Pin, (i>>1) & 0x1);
   digitalWrite(s2Pin, (i>>2) & 0x1);
   currentState[i] = analogRead(mux1); // read mux1 in first 8 array locations
   currentState[i+8] = analogRead(mux2); // read mux2 in last 8 array locations
 }  
}

void saveCurrentState(){  // save the current state for next time
  for(int i=0; i<16; i++){
    lastState[i] = currentState[i];
  }
}

// the value of threshold determines the on / off point
void lookForChange(){
  int ledVal = 0;
  int ledMask = 1;
  for(int i=0; i<16; i++){
    if(currentState[i] < threshold) ledVal |= ledMask;   // add a 1 in the position of sensors under threshold
     ledMask = ledMask << 1;
   }
   if(lastLedVal != ledVal) {   // something has changed
   ledMask = 1;
     for(int i=0; i<16; i++){
       if((ledMask & ledVal) != (ledMask & lastLedVal)){
         if((ledMask & ledVal) == 0) {
           // note off
           if(digitalRead(notesSelect) ){
           midiSend(0xB0, control[i], 0x00);   // turn off control message
           }
           else {
             midiSend(0x80, notes[i], 0x00); // note off
           }
         }
         else{
           // note on
           if(digitalRead(notesSelect) ){
           midiSend(0xB0, control[i], 0x7f);  // turn on control message
           }
           else {
             midiSend(0x90, notes[i], velocityCalculate(currentState[i]) ); // note on
           }
         }
       }
       ledMask = ledMask << 1;
     }
     // Update the trigger LEDs
       gpio_write(ledAddress, ledVal, OLAT);
   }
   lastLedVal = ledVal;        // record current state of LEDs and MIDI notes / messages
}

byte velocityCalculate(int reading){
  int velocity;
    velocity = 95; // default velocity
  // velocity = map(reading, 0, threshold, 127, 0);
  return (byte) velocity;
}

  //  send a MIDI message
 void midiSend(byte cmd, byte data1, byte data2) {
  cmd = cmd | byte(midiChannel);  // merge channel number
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}
//  change the voice
 void programChange(byte cmd, byte data1) {
  cmd = cmd | byte(midiChannel);  // merge channel number
  Serial.write(cmd);
  Serial.write(data1);
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



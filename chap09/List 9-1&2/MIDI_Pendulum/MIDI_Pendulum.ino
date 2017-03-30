
/* Midi Pendulum - Mike Cook Feb 2014
 * send MIDI serial data, for magnetic reed switch sensors
*/
#include "Defines.h"

// #define MIDI_TEST // un comment this line for a test to format the data for the serial monitor

void setup() {
  //  set the states of the I/O pins:
   clearNotes();   // remove any notes playing and zero arrays
   for(int i = 0; i<maxSensors; i++){   // initialise sensor inputs
    pinMode(sensor[i],INPUT); 
   sensorLast[i] = digitalRead(sensor[i]);
   }   


 //  Setup serial / MIDI
#ifdef MIDI_TEST
     Serial.begin(9600);    // Debug speed
#else
     Serial.begin(31250);       // MIDI speed
#endif
   programChange(0xc0, 14);  // Change MIDI voice

}

//********************* MAIN LOOP ***********************************

void loop() {
  int val;
     for(int i = 0; i<maxSensors; i++){   // scan sensor inputs
     val = digitalRead(sensor[i]);
   if( val != sensorLast[i]) { 
       doSensor(i,val);
       sensorLast[i] = val;
      } 
   }  
    } // end loop function
    
//********************* Functions *********************************** 

void doSensor(int s, int v) {  // when a sensor change has been detected

     if( v == LOW ) {          // if we have a make on the sensor
       if(v == bankchangeSensor) {   // and it is the bank change sensor
         stopNotes();          // cycle through the sensor / note mapping banks
         bank++;
         if(bank == 4) bank = 0;
         changeLED();
       }
         // play a note
      noteSend(0x90, note[s][bank], 0x60);
      notePlaying[s] = note[s][bank];
      changeLED();     // change LED colour
       }
     else{  // sensor has released
      noteSend(0x80, note[s][bank], 0x00);   // turn off the note
      notePlaying[s] = 0;
     }
}

void changeLED(){
  static int toggle = 0;
  // toggle LED depending on the bank
   analogWrite(redLED, bankColourR[((bank << 1) + toggle)]);
   analogWrite(greenLED, bankColourG[((bank << 1) + toggle)]);
   analogWrite(blueLED, bankColourB[((bank << 1) + toggle)]);
   toggle ^= 1;   // alternate colour for next time
}

#ifdef MIDI_TEST 
// This is a test so format data for viewing in the serial monitor
 void noteSend(byte cmd, byte data1, byte data2) {
  cmd = cmd | char(midiChannel);  // merge channel number
  Serial.print(((cmd >> 4) & 0xf), HEX);  // to prevent leading Fs being displayed
  Serial.print((cmd & 0xf), HEX);
  Serial.print(" ");
  Serial.print(data1, HEX);
  Serial.print(" ");
  Serial.println(data2, HEX);
  
}
//  change the voice
 void programChange(byte cmd, byte data1) {
   cmd = cmd | char(midiChannel);  // merge channel number
   Serial.print(((cmd >> 4) & 0xf), HEX);  // to prevent leading Fs being displayed
   Serial.print((cmd & 0xf), HEX);
   Serial.print(" ");
   Serial.println(data1, HEX);
}
#else
  // no test so send the stuff out to MIDI
 void noteSend(byte cmd, byte data1, byte data2) {
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
#endif

// stop the notes playing on all notes
void stopNotes() {
   for(int i=0;i<16;i++){
  if (notePlaying[i] !=0) {
      noteSend(0x80, notePlaying[i], 0x00);
     notePlaying[i] = 0;
   }
 }
}

// stop all the notes playing and initialise  arrays
void clearNotes() {
   for(int i=0;i<16;i++){
     noteSend(0x80, note[i][bank], 0x00);
      notePlaying[i] = 0;
 } 
}

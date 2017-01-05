/*
   MIDI to sample player by Mike Cook Nov 2015
   This will take a byte from the serial port and play the sample corrisponding to the MIDI note on number 
  so for example if it recieves a note on 34 it will play the sample file 34.WAV
  Restricted to notes below 100 only
*/
#include <FatReader.h>
#include <SdReader.h>
#include "WaveUtil.h"
#include "WaveHC.h"

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

// ************* Global variables  ********************
        byte incomingByte;
        byte notePlaying; // MIDI note currently playing > 128 == note off
        byte note;
        byte velocity;
        int noteDown = LOW;
        char toPlay[11];     //  string array for file to play 00.WAV to 99999.WAV
 static int indexToWrite=0; // For the recursive name generator 
        int state=0;        // state machine variable 0 = command waiting : 1 = note waitin : 2 = velocity waiting
        int channel = 0;    // MIDI channel to respond to (in this case channel 1) change this to change the channel number
                            // MIDI channel = the value in 'channel' + 1

void sdErrorCheck(void)
{ // freeze if there is an error
  if (!card.errorCode()) return;
  while(1);
}

void setup() {
  Serial.begin(31250);  // MIDI rate
  // Set the output pins for the DAC control.
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
      sdErrorCheck();
  }
    card.partialBlockRead(true);

// Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
     while(1);                            // then 'halt' - do nothing!
  } 
  if (!root.openRoot(vol)) { // Try to open the root directory
    while(1);  // Something went wrong,
  }  
  playfile("init.WAV"); // play a start up sound
}

void loop () {

  if (Serial.available() > 0) {     // read the incoming byte
    incomingByte = Serial.read();
    // add it to the MIDI message
   switch (state){
      case 0:  // looking for a fresh command 
         if (incomingByte== (144 | channel)){   // is it a note on for our channel
         noteDown = HIGH;
         state=1; // move on to look the note to play in the next byte
        }
    if (incomingByte== (128 | channel)){ // is it a note off for our channel
         noteDown = LOW;
         state=1; // move on to look the note to stop in the next byte
        }       
       case 1:
       // get the note to play or stop
       if(incomingByte < 128) {  // have we got a note number
          note=incomingByte;
          state=2;
       }
       else{  // no note number so message is screwed reset to look for a note on for next byte
       state = 0;  // reset state machine as this should be a note number
       }
       break;      
       case 2:
       // get the velocity
       if(incomingByte < 128) {  // is it an off velocity
         playNote(note, incomingByte, noteDown); // fire off the sample
       }
         state = 0;  // reset state machine to start            
     }
  }
}

void playNote(byte note, byte velocity, int down){
  // if velocity = 0 on a 'Note ON' command, treat it as a note off
  if ((down == HIGH) && (velocity == 0)){
      down = LOW; 
      }
  if(down == LOW && notePlaying == note) { 
      wave.stop(); // stop it if it is the current note 
      notePlaying = 255; // indicate no note is playing
     }
  if(down == HIGH) {  // play a sample with the file name based on the note number   
    makeName(note,0);  // generate file name in global array toPlay
    notePlaying = note; // save note number for future stop testing
    playfile(toPlay); // play it
       } 
 }

void makeName(int number, int depth){  // generates a file name 0.WAV to 9999.WAV supressing leading zeros
  if(number > 9) {
    makeName(number / 10, ++depth); // recursion
    depth--;
    number = number % 10;   // only have to deal with the next significant digit of the number
     }
  toPlay[indexToWrite] = (number & 0xf) | 0x30;
  indexToWrite++;
  if(depth > 0) return; // return if we have more levels of recursion to go
  else {  // finish off the string with the wave extesion
     toPlay[indexToWrite] = '.';
     toPlay[1+indexToWrite] = 'W';
     toPlay[2+indexToWrite] = 'A';
     toPlay[3+indexToWrite] = 'V';
     toPlay[4+indexToWrite] = '\0'; // terminator
     indexToWrite = 0; // reset pointer for next time we enter
  }
}

void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) wave.stop();   // something is already playing, so stop it        
  // look in the root directory and open the file
     f.open(root, name);
     if (!wave.create(f)) return; // Not a valid WAV
     wave.play();   // start playback
}


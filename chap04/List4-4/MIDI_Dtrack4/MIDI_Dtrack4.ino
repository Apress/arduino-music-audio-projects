/* MIDI Double Track 3 - Mike Cook
 * for the Teensy board
 * ----------------- 
 * listen for MIDI note on/off data,
 * and plays it on the next track up
 * pan this channel hard left and the other hard right
 * send a third note panned in the middle
*/

//variables setup
  const int led = 13;
  long time;
  boolean noteDown = LOW;
  const byte ourChannel = 1;  // MIDI channel to respond to (in this case channel 1) 

//setup: declaring inputs and outputs 
void setup() { 
  pinMode(led, LOW);
  time = millis() + 2000;
// set up call back channels
 usbMIDI.setHandleNoteOff(doNoteOff);
 usbMIDI.setHandleNoteOn(doNoteOn);
 usbMIDI.setHandleVelocityChange(doVelocityChange);
 usbMIDI.setHandleControlChange(doControlChange);
 usbMIDI.setHandleProgramChange(doProgramChange);
 usbMIDI.setHandleAfterTouch(doAfterTouch);
 usbMIDI.setHandlePitchChange(doPitchChange);
// set pan for the three channels
 usbMIDI.sendControlChange(10, 0, ourChannel); // MSB
 usbMIDI.sendControlChange(42, 0, ourChannel); // LSB
 usbMIDI.sendControlChange(10, 127, (ourChannel+1) & 0xf); // MSB
 usbMIDI.sendControlChange(42, 127, (ourChannel+1) & 0xf); // LSB
 usbMIDI.sendControlChange(10, 64, (ourChannel+2) & 0xf); // MSB
 usbMIDI.sendControlChange(42, 0, (ourChannel+2) & 0xf); // LSB
}

//loop: wait for serial data, and interpret the message 
void loop () {
  usbMIDI.read();
 }
  
  // call back functions basically echo most stuff
  void doVelocityChange(byte channel, byte note, byte velocity){
    usbMIDI.sendPolyPressure(note, velocity, channel);
  }
  
  void doControlChange(byte channel, byte control, byte value){
    usbMIDI.sendControlChange(control, value, channel);
  }
  
  void doProgramChange(byte channel, byte program){
    usbMIDI.sendProgramChange(program, channel);
  }
  
  void doAfterTouch(byte channel, byte pressure){
    usbMIDI.sendAfterTouch(pressure, channel);
  }
  
  void doPitchChange(byte channel, int pitch){
    usbMIDI.sendPitchBend(pitch, channel);
  }

  void doNoteOn(byte channel, byte note, byte velocity){
    digitalWrite(led, HIGH);
    usbMIDI.sendNoteOn(note, velocity, channel);
    if( channel == ourChannel){ // pick out the note we are looking for
      doNote(note, velocity, true);
    }
  }
  void doNoteOff(byte channel, byte note, byte velocity){
    digitalWrite(led, LOW);
    usbMIDI.sendNoteOn(note, velocity, channel);
     if( channel == ourChannel){ // pick out the note we are looking for
      doNote(note, velocity, false);
    }   
  }

void doNote(byte note, byte velocity, int down){
  // if velocity = 0 on a 'Note ON' command, treat it as a note off
  if ((down == HIGH) && (velocity == 0)){
      down = LOW; 
  }
  // send out this note message 
  if(down == LOW){ // note off
    usbMIDI.sendNoteOff(note, velocity, (ourChannel + 1) & 0xf); 
    usbMIDI.sendNoteOff(note, velocity, (ourChannel + 2) & 0xf);
  }
  else { // note on
    usbMIDI.sendNoteOn(note, velocity, (ourChannel + 1) & 0xf); 
    float v = (float)velocity * 0.75;
    usbMIDI.sendNoteOff(note -12,(byte)v, (ourChannel + 2) & 0xf);
  }
}


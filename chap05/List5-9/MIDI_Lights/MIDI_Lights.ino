// MIDI Light Show - Mike Cook
#include <MIDI.h>

byte led[] = {2,3,4,5,6,7,8,9};
// pitch range to respond to
byte minPitch = 60, maxPitch = 68;
void setup()
{
  MIDI.begin(1);
  for(int i=0; i<10;i++){
  pinMode(led[i],OUTPUT);
  }
  
  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(HandleNoteOn);  
  MIDI.setHandleNoteOff(HandleNoteOff);
}


void loop() {
  MIDI.read();
}

void HandleNoteOn(byte channel, byte pitch, byte velocity){
   if(pitch >= minPitch && pitch <=maxPitch ){
     if(velocity == 0) digitalWrite(led[pitch - minPitch], LOW); 
        else digitalWrite(led[pitch - minPitch], HIGH);
   }
 }
 
void HandleNoteOff(byte channel, byte pitch, byte velocity){
     if(pitch >= minPitch && pitch <= maxPitch){
     digitalWrite(led[pitch - minPitch], LOW);
   }
 }


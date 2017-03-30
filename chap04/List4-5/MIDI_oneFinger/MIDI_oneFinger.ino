/* Midi One Finger Wonder - Mike Cook
 * ----------------- 
 * Generates triads, 
*/

//variables setup
  boolean noteDown = LOW;
  byte channel = 0;  // MIDI channel to respond to 
                    // MIDI channel = the value in 'channel' + 1
  boolean bass = true, enableTriad = true; 
  byte triad = 0;
  const int bassPin = 12, triadPin = 10;
  const int triadPin1 = 8, triadPin2 = 6;
  // Major, minor, diminished, augmented
  byte thirds [] = {4, 3, 3, 4};
  byte fifths [] = {7, 7, 6, 8};
  
//setup: declaring inputs and outputs and begin serial 
void setup() { 
  pinMode(bassPin, INPUT_PULLUP);
  pinMode(triadPin, INPUT_PULLUP);
  pinMode(triadPin1, INPUT_PULLUP);
  pinMode(triadPin2, INPUT_PULLUP);
  Serial.begin(31250); //start serial with MIDI baud rate  
}

//loop: wait for serial data, and interpret the message 
void loop () {
  checkIn(); // see if anything has arrived at the input
  getControls(); // get switch values
  }
  
void getControls(){
  bass = digitalRead(bassPin);
  triad = (digitalRead(triadPin1) & 0x1) | ((digitalRead(triadPin2) & 0x1) << 1);
  enableTriad = digitalRead(triadPin);
}

void doNote(byte note, byte velocity, int down){
  // if velocity = 0 on a 'Note ON' command, treat it as a note off
  if ((down == HIGH) && (velocity == 0)){
      down = LOW; 
  }
  // send the other notes of the triad 
  if(down == LOW) { // note off
   if(enableTriad){ 
      noteSend(0x80,note+thirds[triad],velocity);
      noteSend(0x80,note+fifths[triad],velocity);
      if(bass) noteSend(0x80,note-12,velocity);
     }
    }
  else{ // note on
   if(enableTriad){
      noteSend(0x90,note+thirds[triad],velocity);
      noteSend(0x90,note+fifths[triad],velocity);
      if(bass) noteSend(0x90,note-12,velocity);
     }    
  }
}

 void noteSend(char cmd, char data1, char data2) {
  cmd = cmd | char(channel);  // next channel number up
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

void checkIn(){
  static byte note = 60;
  static byte state=0; // state machine variable
  if (Serial.available() > 0) {
    // read the incoming byte:
   byte incomingByte = Serial.read();
   Serial.write(incomingByte);
 
   switch (state){
      case 0:
    // look for as status-byte, our channel, note on
         if (incomingByte == ( 0x90 | channel)){  // read only one channel
            noteDown = HIGH;
            state=1;
         }
    // look for as status-byte, our channel, note off
         if (incomingByte == (0x80 | channel)){   // read only one channel 
            noteDown = LOW;
            state=1;
            }
        break;
       case 1:
       // get the note to play or stop
          if(incomingByte < 128) {
             note=incomingByte;
             state=2;
          }
          else {
            state = 0;  // reset state machine as this should be a note number
            }
        break;      
       case 2:
       // get the velocity
          if(incomingByte < 128) {
              doNote(note, incomingByte, noteDown); // do something with the note on message
             }
          state = 0;  // reset state machine to start            
     }
  }
}

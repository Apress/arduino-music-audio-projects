/* Midi Echo 1 - Mike Cook
 * ----------------- 
 * Echoes a note on the same channel after a delay 
*/

#define bLength 20

//variables setup
  boolean noteDown = LOW;
  byte channel = 0; // MIDI channel = the value in 'channel' + 1 
  static int state=0; // state machine variable 0 = command waiting                   
 // buffer for delayed notes                   
 unsigned long time [bLength];
 byte storeAction[bLength];
 byte storeNote[bLength];
 byte storeVel[bLength];
 unsigned long eDelay = 800;
 
//setup: declaring inputs and outputs and begin serial 
void setup() { 
  Serial.begin(31250); //start serial with MIDI baud rate  
}

//loop: wait for serial data, and interpret the message 
void loop () {
  checkIn(); // see if anything has arrived at the input
  if(state == 0) checkOut(); // see if we need to send anything out
  eDelay = analogRead(0); // read delay value
  }
  
void checkIn(){
  static byte note = 60; 
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
         // look for any after touch, or program message
         if ((incomingByte & 0xE0) == 0xC0){ 
            state=4;  // just wait for the data      
         }
         // look for any control or polyphonic after touch
         if ((incomingByte & 0xE0) == 0xA0){ 
            state=3;  // just wait for two bytes of data      
         }
         // look for any pitch wheel or Channel Mode data
         if ((incomingByte & 0xF0) == 0xA0 || (incomingByte & 0xF0) == 0xB0){ 
            state=3;  // just wait for two bytes of data      
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
              doNote(note, incomingByte, noteDown); // do something withh the note on message
          }
          state = 0;  // reset state machine to start  
         break;
       case 3: // first of two bytes to discard
       state = 4; // next byte to discard
         break;       
       case 4: // data to discard
       state = 0; // reset state machine
     }
  }
}

void doNote(byte note, byte velocity, int down){
  // if velocity = 0 on a 'Note ON' command, treat it as a note off
  if ((down == HIGH) && (velocity == 0)){
      down = LOW; 
  }
  // do something with the note message 
  
  if(down == LOW) { // save the note off in a buffer
     bufferSave(0x80,note,velocity);
  }
  else{ // save the note on in a buffer
    bufferSave(0x90,note,velocity);
  }
}
void noteSend(byte cmd, byte data1, byte data2) {
  cmd = cmd | ((channel+1) & 0xf);  // next channel number up
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

void bufferSave(byte action, byte note, byte vel){
  // search for a free space
  int place = 0;
  while( time[place] !=0 && place < bLength) place++;
  if(place < bLength){ // found one
    time[place] = millis() + eDelay;
    storeAction[place] = action;
    storeNote[place] = note;
    storeVel[place] = vel;
  }
}

void checkOut(){ // see if we need to send anything from the buffer
  for( int place=0; place < bLength; place++){
    if(time[place] !=0 && millis() > time[place]){ // time to send something out
     noteSend(storeAction[place], storeNote[place], storeVel[place]);
     time[place] = 0; // wipe buffer entry
    }
  }
}

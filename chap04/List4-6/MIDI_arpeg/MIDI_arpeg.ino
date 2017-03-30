/* Midi arpeggiator - Mike Cook
 * ----------------- 
 *  Repeats all notes held down note
 * interval controlled by pot on A0
*/

#define bLength 40
 //variables setup
   boolean noteDown = LOW;
   byte channel = 0;     // MIDI channel = the value in 'channel' + 1  
   byte state=0;        // state machine variable 0 = command waiting :
                      // 1 = note waiting : 2 = velocity waiting   
 // buffer for delayed notes                   
   byte storeNote[bLength];
   byte storeVel[bLength];
   unsigned long aDelay = 300;
   unsigned long time = 0;
 
//setup: declaring inputs and outputs and begin serial 
void setup() { 
  Serial.begin(31250); //start serial with MIDI baud rate
  time = millis() +aDelay; 
 }

//loop: wait for serial data, and interpret the message 
void loop () {
  checkIn(); // see if anything has arrived at the input
  if(state == 0) checkOut(); // see if we need to send anything out
  aDelay = analogRead(0); // read delay value
  }
  
void checkIn(){
  static byte note = 60;
 
  if (Serial.available() > 0) {
    // read the incoming byte:
    byte incomingByte = Serial.read();
    Serial.write(incomingByte); // make thru everything received

   switch (state){
      case 0:
    // look for a status-byte, our channel, note on
         if (incomingByte == ( 0x90 | channel)){  // read only one channel
            noteDown = HIGH;
            state=1;
         }
    // look for a status-byte, our channel, note off
         if (incomingByte == (0x80 | channel)){   // read only one channel 
            noteDown = LOW;
            state=1;
         }
      // look for as after touch, our channel
         if (incomingByte == (0xD0 | channel)){   // read only one channel 
            state=3;        
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
       
       case 3: // aftertouch data
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
  
  if(down == LOW) { // remove the note from the buffer
     bufferRemove(note,velocity);
  }
  else{ // save the note on in a buffer
    bufferSave(note,velocity);
  }
}
void noteSend(byte cmd, byte data1, byte data2) {
  cmd = cmd | channel;  // add channel
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

void bufferSave(byte note, byte vel){
  // search for a free space
  int place = 0;
  while( storeNote[place] !=(byte)0 && place < bLength) place++;
  if(place < bLength){ // found one
    storeNote[place] = note;
    storeVel[place] = vel;
    time = millis() + aDelay; // reset arpeggiator timing
  }
}

void bufferRemove(byte note, byte vel){
  // search for the note
  int place = 0;
  while( storeNote[place] != note && place < bLength) place++;
  if(place < bLength){ // found it
    noteSend(0x80, storeNote[place], 0); // stop note from sounding
    storeNote[place] = 0;
  }
}

void checkOut(){ // see if we need to send anything from the buffer
 static int place =0;
 int count = 0;
 if(millis() > time){
   place++;
   if(place >= bLength) place = 0;
   time = millis() + aDelay;
    while(storeNote[place] == 0 && count < bLength){
      place++;
      if(place >= bLength) place = 0;
      count++;
     }
 if(count < bLength) { // found next note to output
     noteSend(0x80, storeNote[place], 0); // turn previous note off
     noteSend(0x90, storeNote[place], storeVel[place]); // trigger note again
   }
 }
}

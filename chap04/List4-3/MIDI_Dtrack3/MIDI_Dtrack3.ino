/* MIDI Double Track 3 - Mike Cook
 * ----------------- 
 * listen for MIDI note on/off data,
 * and plays it on the next track up
 * pan this channel hard left and the other hard right
 * send a third note panned in the middle
*/

//variables setup
  boolean noteDown = LOW;
  byte channel = 0;  // MIDI channel to respond to (in this case channel 1) 
                    //change this to change the channel number
                   // MIDI channel = the value in 'channel' + 1

//setup: declaring inputs and outputs and begin serial 
void setup() { 
  Serial.begin(31250); //start serial with MIDI baud rate
// set pan for the three channels
  controlSend(10, 0, channel); // MSB
  controlSend(42, 0, channel); // LSB
  controlSend(10, 127, (channel+1) & 0xf); // MSB
  controlSend(42, 127, (channel+1) & 0xf); // LSB
  controlSend(10, 64, (channel+2) & 0xf); // MSB
  controlSend(42, 0, (channel+2) & 0xf); // LSB
}

//loop: wait for serial data, and interpret the message 
void loop () {
  checkIn(); // see if anything has arrived at the input
 }
  
void checkIn(){
  static byte note = 60;
  static byte state=0; // state machine variable 0 = command waiting 
                       //: 1 = note waiting : 2 = velocity waiting
  if (Serial.available() > 0) {
    // read the incoming byte:
   byte incomingByte = Serial.read();
   Serial.write(incomingByte); // act as a MIDI THRU

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
              doNote(note, incomingByte, noteDown);
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
  // send out this note message 
  if(down == LOW){ // note off
   noteSend(0x80,note,velocity, 0x1);
   noteSend(0x80,note-12,velocity, 0x2); 
  }
  else { // note on
    noteSend(0x90,note,velocity, 0x1);
    float v = (float)velocity * 0.75;
    noteSend(0x90,note-12,(byte)v, 0x2); // send third note
  }
}
 void noteSend(byte cmd, byte data1, byte data2, byte offset) {
  cmd = cmd | byte((channel + offset) & 0xf);  // next channel number up
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

void controlSend(byte CCnumber, byte CCdata, byte CCchannel) {
  CCchannel |= 0xB0; // convert to Controller message
  Serial.write(CCchannel);
  Serial.write(CCnumber);
  Serial.write(CCdata);
}

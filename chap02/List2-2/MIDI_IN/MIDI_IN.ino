/* Midi input - Mike Cook
 * ----------------- 
 * listen for MIDI serial data, 
*/

//variables setup
  byte channel = 1;  // MIDI channel to respond to (in this case channel 2) change this to change the channel number
                    // MIDI channel = the value in 'channel' + 1

//setup: declaring inputs and outputs and begin serial 
void setup() { 

  pinMode(13,OUTPUT);       // LED to light up
  digitalWrite(13,LOW);     // Turn LED off
  Serial.begin(31250); //start serial with MIDI baud rate  
}

//loop: wait for serial data, and interpret the message 
void loop () {
  checkIn(); // see if anything has arrived at the input
  }
  
void checkIn(){
  static byte state=0;        // state machine variable 0 = command waiting : 1 = note waitin : 2 = velocity waiting
  
 static byte note = 60;
 static boolean noteDown = LOW;
 
  if (Serial.available() > 0) {
    // read the incoming byte:
    byte incomingByte = Serial.read();

   switch (state){
      case 0:
    // look for as status-byte, our channel, note on
         if (incomingByte== ( 0x90 | channel)){  // read only one channel
            noteDown = HIGH;
            state=1;
         }
    // look for as status-byte, our channel, note off
         if (incomingByte== (0x80 | channel)){   // read only one channel 
            noteDown = LOW;
            state=1;
        }
        
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
     }
  }
}

void doNote(byte note, byte velocity, int down){
  // if velocity = 0 on a 'Note ON' command, treat it as a note off
  if ((down == HIGH) && (velocity == 0)){
      down = LOW; 
  }
  // do something with the note message 
  // this just toggles Pin 13 and ignores the note value and velocity
 if(down == LOW) digitalWrite(13, LOW); else  digitalWrite(13, HIGH);

}

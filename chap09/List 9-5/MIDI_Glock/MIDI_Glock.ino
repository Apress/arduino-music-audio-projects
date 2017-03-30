/* Midi Glock - Mike Cook Feb 2014
 * listen for MIDI serial data, and fire solenoids for individual notes
*/
  byte incomingByte;
  byte note;
  byte velocity;
  int state = 0;  // state machine variable
  byte baseNote = 60;  // lowest note
  // use different values of baseNote to select the MIDI octave
  // 24 for octave  1 -- 36 for octave  2 -- 48 for octave  3 -- 60 for octave  4 -- 72 for octave  5
  // 84 for octave  6 -- 96 for octave  7
  
// play only notes in the key of C (that is no sharps or flats) define pin numbers:-
  byte playArray[] =    { 2,  0,  3,  0,  4,  5,  0,  6,  0,  7,  0,  8,  9 };
// corresponding  to note 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48 - for base note = 36 or C2
int channel = 0; // MIDI channel to respond to (in this case channel 1) chnage this to change the channel number

unsigned long noteOnTime[13];
unsigned long firingTime = 40; // time to hold the solenoid on

void setup() { 
  for(int i =0 ; i<13; i++){
  if(playArray[i] !=0){
     pinMode(playArray[i],OUTPUT);        // declare the solenoid's pins as outputs
     digitalWrite(playArray[i], LOW);     // set to off
     }
  }
  Serial.begin(31250);    //start serial with MIDI baudrate     
}

//loop: wait for serial data, and interpret the message 
void loop () {
 // check to see if we need to turn any pin off
 for(int offCheck = 0; offCheck < 13; offCheck++){
   if((noteOnTime[offCheck] != 0) && (millis() - noteOnTime[offCheck] > firingTime) ){
      noteOnTime[offCheck] = 0;
     if(playArray[offCheck] !=0) digitalWrite(playArray[offCheck], LOW);
   }
 }
 // see if anything new has arrived
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
   switch (state){
      case 0:
    // look for as status-byte, our channel, note on
    if (incomingByte== (144 | channel)){ 
         state=1;
        }
      // consider follow on mode
       if(incomingByte< 128){
        // use same noteDown as last time
        note=incomingByte;
        state=2;
       }
       
       case 1:
       // get the note to play
       if(incomingByte < 128) {
          note=incomingByte;
          state=2;
       }
       else{
       state = 0;  // reset state machine as this should be a note number
       }
       break;
       
       case 2:
       // get the velocity
       if(incomingByte < 128) {
         playNote(note, incomingByte); // fire off the solenoid
       }
         state = 0;  // reset state machine to start            
     }
  }
}

void playNote(byte note, byte velocity){
  byte playPin;
  if(velocity != 0) {  //only fire solenoids with non zero velocity
 //since we can't play all notes we only action some notes
  if(note >= baseNote && note <= (baseNote + 13)){
    playPin = playArray[note - baseNote]; // to get a pin number between 2 and 9
    if(playPin != 0) { // if it is a note in the scale
     digitalWrite(playPin, HIGH); // play it if it is one of our notes
     noteOnTime[note - baseNote] = millis(); // time we turned the note on
    }
  }
 } 
}


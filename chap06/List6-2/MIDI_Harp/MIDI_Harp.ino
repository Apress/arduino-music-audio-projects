// MIDI Harp by Mike Cook

   // MIDI values
  // play only notes in the key of C (that is no sharps or flats) 
 // define pin numbers:-
  byte midiString[] =  { 2, 0, 3, 0, 4, 5, 0, 6, 0, 7, 8, 0,  // 55 to 66
                      9, 0, 10, 0, 11, 12, 0, 13, 0, 14, 15, 0, // 67 to 78
                      16,  0, 17,  0 };    // 79 to 82

  const int lowString =55, highString = 82;
  byte incomingByte;
  byte note;
  byte velocity;
  int state=0;       // state machine variable 0 = command waiting
                    // 1 = note waiting : 2 = velocity 
  int channel = 0; // MIDI channel to respond to (in this case channel 1) 
                  // change this to change the channel number
                 // MIDI channel = the value in 'channel' + 1

void setup() {
  Serial.begin(31250);  // for MIDI speed
    // make all bits outputs and low
  for(int i=2; i<20; i++){
    pinMode(i, OUTPUT); // make the pin an output
    digitalWrite(i, LOW); // set it low
    }
   while (!Serial) { } // needed for Leonardo only 
   delay(20);
   digitalWrite(19, HIGH); // remove motor flip flop clear
} 

void loop() {
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
   switch (state){
      case 0:
    // look for as status-byte, our channel, note on
    if (incomingByte == (0x90 | channel)){ 
         state=1;         
        }       
        break;
      case 1:
       // get the note to play or stop
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
         playNote(note, incomingByte); // turn the servo
       }
         state = 0;  // reset state machine to start       
     }
  }
}

void playNote(byte note, byte velocity){
    // if velocity = 0 on a 'Note ON' command, treat it as a note off
    if (velocity == 0) return;
   if(note >= lowString && note <= highString){ // is it in the range of our strings and to play
      note -= lowString; // start the MIDI notes in our range from zero 
      if(midiString[note] !=0) { // if we have a string for this
         // move the motor
         if( digitalRead(midiString[note]) == HIGH) { // toggle output pin
            digitalWrite(midiString[note], LOW);
         }
        else {
          digitalWrite(midiString[note], HIGH);
         }  // end of toggle the output pin
      }  // end of if we have a string for this
   }  // end of is it in the range of our strings and to play
}  // end of function

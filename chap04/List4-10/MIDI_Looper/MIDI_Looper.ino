/* MIDI Looper - Mike Cook
 * ----------------- 
 * records MIDI input then plays it out continuously 
*/

#define bLength 100

//variables setup
  boolean noteDown = LOW;
  const int recordLed = 7, replayLed =5; // onboard LED
  byte channel = 0; // MIDI channel = the value in 'channel' + 1
 int state=0;      // state machine variable 0 = command waiting : 1 = note waiting : 2 = velocity waiting
 int place;
 // buffer for delayed notes                   
 unsigned long time [bLength];
 byte storeAction[bLength];
 byte storeNote[bLength];
 byte storeVel[bLength];
 unsigned long startTime=0, endTime=0, bufferTime=0;
 boolean recording = false, playback = false, empty = false;
 const byte recordPin = 12, stopPin = 10;
 
//setup: declaring inputs and outputs and begin serial 
void setup() { 
  pinMode(recordLed,OUTPUT); // LED to light up when recording
  pinMode(replayLed,OUTPUT); // LED to light up when replaying
  digitalWrite(recordLed,LOW);     // Turn LED off
  digitalWrite(replayLed,LOW);     // Turn LED off
  pinMode(recordPin, INPUT_PULLUP);
  pinMode(stopPin, INPUT_PULLUP);
  Serial.begin(31250); //start serial with MIDI baud rate   
}

//loop: wait for serial data, and interpret the message 
void loop () {
  checkIn(); // see if anything has arrived at the input
  if(state == 0) checkOut(); // see if we need to send anything out
  getControls(); // read switches
  }
  
void getControls(){
  static long debounce = 0;
  static long debounceTime = 25;
  boolean rec = !digitalRead(recordPin);
  if(!recording && rec && digitalRead(stopPin) && millis() > debounce){
    recording = true;
    debounce = millis() + debounceTime;
    if(playback){
       playback = false;
       wipeBuffer();
    }
    startTime = millis();
    digitalWrite(recordLed, HIGH);
  }
  if(recording && !rec && millis() > debounce){ //debounce
     recording = false;
     playback = true;
    debounce = millis() + debounceTime;
    time[place] = 0; // mark buffer end
     bufferTime = millis() - startTime;
     digitalWrite(recordLed, LOW);
     prepBuffer();
   }
   if(!digitalRead(stopPin)){
      recording = false;
      playback = false;
      wipeBuffer();
    }
}

void wipeBuffer(){
  for(int i =0; i<bLength; i++){
    time[i] = 0L;
  }
  place = 0;
}

void prepBuffer(){ // set buffer for next time it should play
  int i=0;
  while(time[i] != 0){
    time[i] += bufferTime;
    i++;
  }
  place = 0;
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
              doNote(note, incomingByte, noteDown); // do something with the note on message
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
  
  if(down == LOW) { // record the note off in the buffer
    bufferSave(0x80,note,velocity);
  }
  else {
    bufferSave(0x90,note,velocity);
  }
 }

void noteSend(byte cmd, byte data1, byte data2) {
  cmd = cmd | channel;

  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

void bufferSave(byte action, byte note, byte vel){
  // place note in next position in buffer
  if(recording){
     time[place] = millis();
     storeAction[place] = action;
     storeNote[place] = note;
     storeVel[place] = vel;
     place++; // for next time
       if(place >= bLength) { // come to the end of the buffer
       // make it start recording again because record key is still held
       recording = false;
       playback = true;
       }
  }
}

void checkOut(){ // see if we need to send anything from the buffer
  if(playback && time[0] != 0L) {
    digitalWrite(replayLed,HIGH);
    if(millis() > time[place]){
     noteSend(storeAction[place], storeNote[place], storeVel[place]);
     time[place] += bufferTime; // set buffer entry for next play
     place++; // point at next play
     if(time[place] ==0) place = 0; // wrap pointer round 
    }
    digitalWrite(replayLed,LOW);
  }
}

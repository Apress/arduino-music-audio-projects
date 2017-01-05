/* Midi Echo 2 - Mike Cook
 * ----------------- 
 * Echoes a note on the same channel after a delay
 * Up to four echoes with individual time control
 * and mode control for the timing. 
*/

#define bLength 80

//variables setup
  boolean noteDown = LOW;
  const int led = 13;
  byte channel = 0;  // MIDI channel = the value in 'channel' + 1                  
  static int state=0; // state machine variable 0 = command waiting 
 // buffer for delayed notes                   
 unsigned long time [bLength];
 byte storeAction[bLength];
 byte storeNote[bLength];
 byte storeVel[bLength];
 unsigned long eDelay1 = 800, eDelay2 = 800, eDelay3 = 800, eDelay4 = 800;
 boolean echoChan = false, delay1 = false, delay2 = false,  delay3 = false;
 boolean delay4 = false, sameDelay = false;
 const byte echoChanPin = 12;
 const byte delay1Pin = 10;
 const byte delay2Pin = 8;
 const byte delay3Pin = 6;
 const byte delay4Pin = 4;
 const byte sameDelayPin = 2; // gives all echos same delay from one pot
 
//setup: declaring inputs and outputs and begin serial 
void setup() { 
  pinMode(led,OUTPUT);       // LED to light up
  pinMode(echoChanPin, INPUT_PULLUP);
  pinMode(delay1Pin, INPUT_PULLUP);
  pinMode(delay2Pin, INPUT_PULLUP);
  pinMode(delay3Pin, INPUT_PULLUP);
  pinMode(delay4Pin, INPUT_PULLUP);
  pinMode(sameDelayPin, INPUT_PULLUP);
  digitalWrite(led,LOW);     // Turn LED off
  Serial.begin(31250); //start serial with MIDI baud rate  
}

//loop: wait for serial data, and interpret the message 
void loop () {
  checkIn(); // see if anything has arrived at the input
  if(state == 0)digitalWrite(led,LOW); else digitalWrite(led,HIGH);
    if(state == 0) checkOut(); // see if we need to send anything out
    else {
      getControls();
    }
  }
  
void getControls(){
  // get analogue delays
    eDelay1 = analogRead(0)<< 1;
    eDelay2 = analogRead(1)<< 1;
    eDelay3 = analogRead(2)<< 1;
    eDelay4 = analogRead(3)<< 1;
 // get digital controls
   echoChan = digitalRead(echoChanPin);
   delay1 = digitalRead(delay1Pin);
   delay2 = digitalRead(delay2Pin);
   delay3 = digitalRead(delay3Pin);
   delay4 = digitalRead(delay4Pin);
   sameDelay = digitalRead(sameDelayPin);
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
  byte action = 0x90;
  if(down == LOW) { // save the note off in a buffer
  action = 0x80;
  }
  if (sameDelay){// use one pot for all delays
    if(delay1) bufferSave(action,note,velocity,eDelay1);
    if(delay2) bufferSave(action,note,velocity,eDelay1*2);
    if(delay3) bufferSave(action,note,velocity,eDelay1*3);
    if(delay4) bufferSave(action,note,velocity,eDelay1*4);

  }
  else { // use a separate pot for each delay
    if(delay1) bufferSave(action,note,velocity,eDelay1);
    if(delay2) bufferSave(action,note,velocity,eDelay1+eDelay2);
    if(delay3) bufferSave(action,note,velocity,eDelay1+eDelay2+eDelay3);
    if(delay4) bufferSave(action,note,velocity,eDelay1+eDelay2+eDelay3+eDelay4);
    }
}

void noteSend(byte cmd, byte data1, byte data2) {
  if(echoChan) {
  cmd = cmd | channel;
  }
  else{
  // cmd = cmd | 9; // send on drum channel
  cmd = cmd | ((channel+1) & 0xf);  // next channel number up
  }
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

void bufferSave(byte action, byte note, byte vel, long int echo){
  // search for a free space
  int place = 0;
  while( time[place] !=0 && place < bLength) place++;
  if(place < bLength){ // found one
    time[place] = millis() + echo;
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

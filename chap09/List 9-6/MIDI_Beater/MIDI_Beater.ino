// MIDI Beater by Mike Cook Feb 2014
#include <Servo.h> 

Servo beatServo[8];  // create servo object to control a servo 

byte midiActivity = 7; // activity LED
byte servoPin[] = {2, 3, 4, 8, 9, 10, 14, 15};  // define what pins the servo is on
byte ledPins[] = {5, 6, 18, 17, 11, 12, 13, 16};  // defines the pins the LEDs are on
byte servoPos[] = {0, 0, 0, 0, 0, 0, 0, 0}; // to determine  if LED should be on or off
byte servoOffPos[] = {0, 0, 0, 0, 0, 0, 0, 0}; // position the note off should go to
byte lowestServo = 48; // MIDI note for lowest servo
byte lowestServoOff = 28; // MIDI note for lowest servo off
int channel = 0;

void setup() 
{ 
  for(int i =0; i<8; i++){
  beatServo[i].attach(servoPin[i]);
  pinMode(ledPins[i], OUTPUT);
  digitalWrite(ledPins[i], HIGH);   // turn off LED
  }
   for(int i =0; i<8; i++){
     beatServo[i].write(servoOffPos[i]);  // positions the servos to the off point
    } 
   pinMode(midiActivity, OUTPUT);
   digitalWrite(midiActivity, HIGH); // turn off MIDI for us light   
   Serial.begin(31250);  // for MIDI
} 

void loop() 
{ 
  checkMIDI();
} 

void checkMIDI() {
   static int state=0; 
   static boolean noteDown = LOW; 
   static byte note;
   byte incomingByte;

  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
   switch (state){
      case 0:
    // look for as status-byte, our channel, note on
    if (incomingByte == (144 | channel)){ 
         noteDown = HIGH;
         state=1;         
        }       
    // look for a status-byte, our channel, note off
    if (incomingByte == (128 | channel)){ 
         noteDown = LOW;
         state=1;
        }
      if( ((incomingByte & 0xf) == channel) && (state == 1)) // light up command for us
        digitalWrite(midiActivity, LOW); // turn on MIDI for us light
        else
        digitalWrite(midiActivity, HIGH); // turn off MIDI for us light

        break;
       case 1:
       // get the note to play or stop
       if(incomingByte < 128) {
          note=incomingByte;
          state=2;
       }
       else{
       state = 0;  // reset state machine as this should be a note number
       digitalWrite(midiActivity, HIGH); // turn off MIDI for us light
       }
       break;
       
       case 2:
       // get the velocity
       if(incomingByte < 128) {
         playNote(note, incomingByte, noteDown); // turn the servo
       }
         state = 0;  // reset state machine to start
         digitalWrite(midiActivity, HIGH); // turn off MIDI for us light         
     }
  }
}

void playNote(byte note, byte velocity, int down){
    byte index = note - lowestServo;
    // if velocity = 0 on a 'Note ON' command, treat it as a note off
    if ((down == HIGH) && (velocity == 0)){
      down = LOW; 
       }     
   if(note>= lowestServo && note < lowestServo + 8){ // is it in the range of our servos
   if(down == LOW) velocity = servoOffPos[index];   // make it the off position
    beatServo[index].write(velocity); // make servo move to angle given by velocity
    if(velocity > servoPos[index]) digitalWrite(ledPins[index], LOW); else digitalWrite(ledPins[index], HIGH);
    servoPos[index] = velocity;
     } 
     else { // set off position
     if( (note>= lowestServoOff) && (note < (lowestServoOff + 8)) ){ // is it in the range of our off position control
     if(down == HIGH)servoOffPos[note - lowestServoOff] = velocity;  // set the note off position
     }
   }
 }

/* MIDI Spoon-o-phone - Mike Cook
* simple pull down inputs for piano keys
* Alligator clip defines the key
*/

const byte keyPins[] = { 15,16,17,18,19,13,12,11,10,9,8,7,6,5,4,3,2};                       
//                     Base notes -7 (starting at G)
const byte notes[] =   {0,2,4,5,7,9,11,12,14,16,17,19,21,23,24,
                       26,28,29,31,33,35,36,38,40,41,43,45,47,48};
const int keyThreshold[] = {85, 255, 426, 596, 767, 937, 1024};
const int musicKeyPin = A0; // key select alligator clip input pin
const int numberOfKeys = 17;
boolean keyUp[numberOfKeys];
byte channel = 0;  // MIDI channel to send data on
byte octave = 3; // two below Middle C
byte lookUpOffset, keyOffset, playingKey;

void setup(){
  Serial.begin(31250); // MIDI baud rate
  for(int i=0; i<numberOfKeys; i++){
    pinMode(keyPins[i], INPUT_PULLUP);
    keyUp[i] = true;
  }
  playingKey = findKey(); // 1 to 7 is A to G
  keyOffset = notes[playingKey];
  lookUpOffset = 7+ (octave * 12);
  // setup voice to use
  controlSend(0, 0, channel); // set bank 0 MSB
  controlSend(32, 0, channel); // set bank 0 LSB
  programChange(19); // send voice number - Church organ
}

void loop(){ // look and see if key is down send MIDI message if it is
  boolean key;
  for(int i=0; i<numberOfKeys; i++){
    key = digitalRead(keyPins[i]);
    if(key != keyUp[i]){ // change in key state
      if(keyUp[i]) {
      midiSend(0x90, lookUpNote(i), 127); // note on
     }
      else  { 
      midiSend(0x80, lookUpNote(i), 0); // note off
     }
      keyUp[i] = key; // save the new state of the key
    }
  }
}

void midiSend(byte cmd, byte d1, byte d2){
  cmd = cmd | channel;
  Serial.write(cmd);
  Serial.write(d1);
  Serial.write(d2);
}

byte lookUpNote(int i){
  byte note;
  note = notes[i] + lookUpOffset + keyOffset;
  return note;
}

byte findKey(){ // look up what key we are in
  int i = 0;
  int clipVoltage = analogRead(musicKeyPin);
  while(keyThreshold[i] < clipVoltage) i++;
  i++;
  return (byte) i;
}

void controlSend(byte CCnumber, byte CCdata, byte CCchannel) {
  CCchannel |= 0xB0; // convert to Controller message
  Serial.write(CCchannel);
  Serial.write(CCnumber);
  Serial.write(CCdata);
}

void programChange(byte voice){
  Serial.write((byte)0xC0 | channel);
  Serial.write(voice);
}

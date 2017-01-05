// Theremin  - Mike Cook

boolean playing = false;
const byte channel = 0;
const byte baseNote = 72;

void setup(){
  Serial.begin(31250); // MIDI baud rate
  analogReference(EXTERNAL);
  controlSend(0, 0); // set bank 0 MSB
  controlSend(32, 0); // set bank 0 LSB
  programChange(52); // send voice number
  // set up Pitch bend sensitivity
  controlSend(101, 0);
  controlSend(100, 0); 
  controlSend(6,72); // set to 6 octaves
  controlSend(38, 0); // and zero cents
}

void loop(){
  int av1 = 1027 - analogRead(0);
  int av2 = 1027 - analogRead(1);
  if(av1 <870 && av2 < 870){
     if(!playing)noteOn();
     else {
          trackNote(av2,av1);
     }
  }
  else {
    if(playing) noteOff();
  }
}

void noteOff(){
  playing= false;
  // note off + sustain off
  noteSend(0x80,baseNote,127);
  controlSend(64, 0);
}
void noteOn(){
 // note on + sustain on
  noteSend(0x90,baseNote,127);
  controlSend(64, 127);
  playing = true;
}

int trackNote(int freq, int volume){
  int pb = 0x2000 - (435 - freq);
  sendPB(pb);
  int vel = volume>> 3;
   controlSend(7, vel);
}

 void noteSend(byte cmd, byte data1, byte data2) {
  cmd = cmd | channel;
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

void controlSend(byte CCnumber, byte CCdata) {
  byte CCchannel = channel | 0xB0; // convert to Controller message
  Serial.write(CCchannel);
  Serial.write(CCnumber);
  Serial.write(CCdata);
}

void sendPB(int pb){ // send pitch bend message
  Serial.write( (byte)0xE0 | channel);
  Serial.write( pb & (byte)0x7f);
  Serial.write( (pb>>7) & (byte)0x7f);
}

void programChange(byte voice){
  Serial.write((byte)0xC0 | channel);
  Serial.write(voice);
}

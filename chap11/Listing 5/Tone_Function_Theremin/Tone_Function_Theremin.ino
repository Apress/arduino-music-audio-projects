// Tone function Theremin  - Mike Cook

const byte soundPin = 13;
void setup(){
  analogReference(EXTERNAL);
}

void loop(){
  int av1 = 1027 - analogRead(0); // pitch 
  if(av1 < 870){    // if hand over sensor
     trackNote(av1);
     }
  else {
    noTone(soundPin);
  }
}

int trackNote(int freq){
  int pitch = map(freq, 100, 870, 100, 1000);
  tone(soundPin,pitch);
}


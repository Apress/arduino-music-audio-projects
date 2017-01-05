// Therimin test - Mike Cook

boolean playing = false;
void setup(){
  Serial.begin(9600);
  analogReference(EXTERNAL);
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
  delay(100);
}

void noteOff(){
  playing= false;
  Serial.println("note off + sustain off");
}
void noteOn(){
  Serial.println("note on + sustain on");
  // send note on
  // send sustain
  playing = true;
}

int trackNote(int freq, int volume){
  int pb = 0x2000 - (435 - freq);
  Serial.println(pb,HEX);
  int vel = volume>> 3;
  Serial.println(vel);
}

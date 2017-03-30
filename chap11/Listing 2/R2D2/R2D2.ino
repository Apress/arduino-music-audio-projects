// R2D2
const byte soundPin = 13;
void setup() {
  pinMode(soundPin,OUTPUT);
}

void loop() {
  for(int i; i<50; i++){
     note(random(100,2000));
  }
  delay(2000);
}

void note(int halfPeriod) { 
  for (int i=0; i<20; i++) {
     digitalWrite(soundPin,HIGH);
     delayMicroseconds(halfPeriod);
     digitalWrite(soundPin,LOW);
     delayMicroseconds(halfPeriod);
  } 
}

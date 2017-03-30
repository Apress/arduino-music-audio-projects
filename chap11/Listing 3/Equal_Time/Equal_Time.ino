// Equal Time tones
const byte soundPin = 13;
void setup() {
  pinMode(soundPin,OUTPUT);
}

void loop() {
  for(int i; i<30; i++){
     note(random(100,2000), 90);
  }
  delay(2000);
}

void note(int halfPeriod, long interval) {
  long startTime = millis(); 
  while(millis()-startTime < interval) {
     digitalWrite(soundPin,HIGH);
     delayMicroseconds(halfPeriod);
     digitalWrite(soundPin,LOW);
     delayMicroseconds(halfPeriod);
  } 
}

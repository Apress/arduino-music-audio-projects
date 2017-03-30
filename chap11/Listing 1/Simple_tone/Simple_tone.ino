// Ultra simple tone
const byte soundPin = 13;
int halfPeriod = 1000;
void setup() {
  pinMode(soundPin,OUTPUT);
}

void loop() {
  digitalWrite(soundPin,HIGH);
  delayMicroseconds(halfPeriod);
  digitalWrite(soundPin,LOW);
  delayMicroseconds(halfPeriod); 
}

// MIDI Harp player test
void setup(){
  Serial.begin(9600);
  // make all bits outputs and low
  for(int i=2; i<20; i++){
    pinMode(i, OUTPUT); // make the pin an output
    digitalWrite(i, LOW); // set it low
    }
  while (!Serial) { } // needed for Leonardo only
  Serial.println("Harp test");
  Serial.println("type in the string number");
  delay(20);
  digitalWrite(19, HIGH); // remove motor flip flop clear
}

void loop(){
  int number = Serial.parseInt();
  if(number != 0 ){
    if(number > 17) {
      Serial.println("String number too big");
    }
    else {
      Serial.print("String number ");
      Serial.print(number);
      Serial.println(" moved");
      if( digitalRead(number+1) == HIGH) { // toggle the output pin
          digitalWrite(number+1, LOW);
        }
        else {
          digitalWrite(number+1, HIGH);
        }
    }
  }
}

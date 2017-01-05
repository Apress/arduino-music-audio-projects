/* Soft pot reading example  - Mike Cook
* Linear soft pot wired to:-
* wiper to analogue input 0
* ends to Pins 2 & 3 
* Circular soft pot wired to:-
* wiper to analogue input 1
* ends to Pins 4 & 5
*/

const byte end1a = 2, end1b = 3;
const byte end2a = 4, end2b = 5;
int threshold = 40; // closeness of reading to be valid

void setup(){
  // set up the pot end pins
  pinMode(end1a, OUTPUT);
  pinMode(end1b, OUTPUT);
  pinMode(end2a, INPUT_PULLUP);
  pinMode(end2b, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop(){
  int pot1 = readLin();
  int pot2 = readCir();
  if(pot1 != -1) {
    Serial.print("Linear Pot reading ");
    Serial.println(pot1);
  }
  else {
    Serial.println("Linear Pot not touched");
  }
    if(pot2 != -1) {
    Serial.print("Circular Pot reading ");
    Serial.println(pot2);
  }
  else {
    Serial.println("Circular Pot not touched");
  }
  delay(800); // stop the display going too fast
}

int readLin(){ // returns -1 for an invalid reading
  digitalWrite(end1a, HIGH);
  digitalWrite(end1b, LOW);
  int read1 = analogRead(0); // read one way
  digitalWrite(end1b, HIGH);
  digitalWrite(end1a, LOW);
  int read2 = analogRead(0); // read the other way
  if( abs((read1 + read2)-1023) > threshold ) return -1; else return read1;
}

int readCir(){ // returns -1 for an invalid reading
  pinMode(end2a, INPUT_PULLUP);
  pinMode(end2b, OUTPUT);
  digitalWrite(end2b, LOW);
  if(digitalRead(end2a) == LOW){  // short across pot ends
     pinMode(end2b, INPUT_PULLUP);
     return -1; // not safe to proceed - abandon
  }
  // safe to proceed with reading
  pinMode(end2a, OUTPUT);
  digitalWrite(end2a, HIGH);
  digitalWrite(end2b, LOW);
  int read1 = analogRead(0); // read one way
  digitalWrite(end2b, HIGH);
  digitalWrite(end2a, LOW);
  int read2 = analogRead(0); // read the other way
  // return wiper pins to safe mode
  pinMode(end2a, INPUT_PULLUP);
  pinMode(end2b, INPUT_PULLUP);
  if( abs((read1 + read2)-1023) > threshold ) return -1; else return read1;
}


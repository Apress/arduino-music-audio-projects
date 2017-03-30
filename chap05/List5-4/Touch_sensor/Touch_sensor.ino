// Touch sensor test - Mike Cook

void setup(){
  Serial.begin(9600);
}

void loop(){
  int xPos = measureX();
  int yPos = measureY();
  if(xPos != -1 && yPos !=-1){
  Serial.print("X = ");
  Serial.print(xPos);
  Serial.print(" & Y = ");
  Serial.println(yPos);
  }
  delay(100);
}

int measureX(){
 int v1,v2,v;
  pinMode(14,OUTPUT);
  pinMode(16,OUTPUT);
  digitalWrite(14,LOW);
  digitalWrite(16,LOW);
  v1 = analogRead(A3);
  digitalWrite(14,HIGH);
  digitalWrite(16,HIGH);
  v2 = analogRead(A3);
  if(v1 <60 && v2 > 990) {
  digitalWrite(16,LOW);
  digitalWrite(14,HIGH);
  v = analogRead(A3);
  }
  else {
    /*
    Serial.print(" low reading ");
    Serial.print(v1);
    Serial.print(" high reading ");
    Serial.println(v2);
   */
    v=-1;
  }
  pinMode(16,INPUT);
  digitalWrite(14,LOW);
  pinMode(14,INPUT);
  return v;
}

int measureY(){
 int v1,v2,v;
  pinMode(15,OUTPUT);
  pinMode(17,OUTPUT);
  digitalWrite(15,LOW);
  digitalWrite(17,LOW);
  v1 = analogRead(A0);
  digitalWrite(15,HIGH);
  digitalWrite(17,HIGH);
  v2 = analogRead(A0);
  if(v1 <60 && v2 > 990) {
  digitalWrite(15,LOW);
  digitalWrite(17,HIGH);
  v = analogRead(A0);
  }
  else {
    /*
   Serial.print(" low reading ");
    Serial.print(v1);
   Serial.print(" high reading ");
   Serial.println(v2);
   */
    v=-1;
  }
  pinMode(15,INPUT);
  digitalWrite(17,LOW);
  pinMode(17,INPUT);
  return v;
}

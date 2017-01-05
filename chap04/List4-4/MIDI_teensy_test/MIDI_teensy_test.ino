const int led = 13;  
void setup() { 
  pinMode(led, LOW);
}

//loop: wait for serial data, and interpret the message 
void loop () {
  static byte chan = 0;
  if(usbMIDI.read(1) && usbMIDI.getChannel() == 1){
  int type = usbMIDI.getType();
  if(type == 0) {
    digitalWrite(led,LOW);
   // usbMIDI.sendNoteOff(64, 99, chan);
  }
  if(type == 1){
    digitalWrite(led,HIGH);
        chan +=1;
    if (chan >16) chan =1;
    Serial.println(chan);
   usbMIDI.sendNoteOn(64, 99, chan);
   }
  }
 }


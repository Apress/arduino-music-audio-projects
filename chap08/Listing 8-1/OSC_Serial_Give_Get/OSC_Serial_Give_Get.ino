/*
OSC Serial Give & Get - Mike Cook
Sends out OSC messages and receives them
For PD or Max. See the one line in the 
rxMessage() function to change for PD or Max
*/
#include <OSCBundle.h>
#include <OSCBoards.h>
#include <OSCMessage.h>

// I/O pin mapping
byte inPins[] = {12, 10, 8, 6, 4, 2};
byte outPins[] = {13, 11, 9, 7, 5, 3};
byte numberOfPots = 4;

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
 SLIPEncodedSerial SLIPSerial(Serial);
#endif

void setup() {
  for(int i=0; i<6; i++){
    pinMode(inPins[i], INPUT_PULLUP);
    pinMode(outPins[i], OUTPUT);
    digitalWrite(outPins[i],LOW);
    }
  SLIPSerial.begin(38400);   // set this as high as you can reliably run on your platform
#if ARDUINO >= 100
  while(!Serial)
    ; //Leonardo requirement to establish USB device
#endif
}
   
void loop(){
  // sendAutoMessage(); // simulate analogue signals
  checkAnalogue(); // look at real analog ports
  if(SLIPSerial.available() > 0) rxMessage();
  checkDigital();
  delay(10);
}

void checkDigital(){
  static boolean lastSwitchState [6];
  char messageDigital[] ="/d/3";
  boolean currentSwitchState;
  for(int i = 0; i<6; i++){
     currentSwitchState = digitalRead(inPins[i]);
    if(  currentSwitchState != lastSwitchState[i]){
      lastSwitchState[i] = currentSwitchState;
       messageDigital[3] = char( i | 0x30);
      OSCMessage mssageD(messageDigital);
      mssageD.add((int32_t)currentSwitchState & 1);
      SLIPSerial.beginPacket();  
      mssageD.send(SLIPSerial); // send the bytes to the SLIP stream
      SLIPSerial.endPacket(); // mark the end of the OSC Packet
      mssageD.empty(); // free space occupied by message
    }
  } 
}

void checkAnalogue(){
  static int lastAnalogueValue [6];
  char messageAnalog[] ="/a/5";
  int currentAnalogueReading;
  for(int i=0; i<numberOfPots; i++){
    currentAnalogueReading = analogRead(i);
    if(abs(currentAnalogueReading - lastAnalogueValue[i]) > 2){
      lastAnalogueValue[i] = currentAnalogueReading;
      messageAnalog[3] = char(i + 0x30);
      OSCMessage msg(messageAnalog);
      msg.add((int32_t)currentAnalogueReading);
      // now send the message
      SLIPSerial.beginPacket();  
      msg.send(SLIPSerial); // send the bytes to the SLIP stream
      SLIPSerial.endPacket(); // mark the end of the OSC Packet
      msg.empty(); // free space occupied by message
    }
  }
}

void sendAutoMessage(){
  static int count = 10, ch =0;
  char messageAnalog[] ="/a/5";
  count +=10;
  if(count> 1023) count = 0;
  ch++;
  if(ch>6) ch=0;
  messageAnalog[3] = char(ch + 0x30);
  OSCMessage msg(messageAnalog);
  msg.add((int32_t)count);
  // now send the message
  SLIPSerial.beginPacket();  
  msg.send(SLIPSerial); // send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message
  }
  
void rxMessage(){
  // Max uses OSCBundle and PD uses OSCMessage
// OSCMessage messageIN; // uncomment for PD
 OSCBundle messageIN;  // comment out for PD
 int sizeb =0;
 while(!SLIPSerial.endofPacket() ) {
    if( (sizeb =SLIPSerial.available()) > 0)
    { 
       while(sizeb--){
          messageIN.fill(SLIPSerial.read());
       }
     }
  }
   if(!messageIN.hasError()) { // error free
    messageIN.route("/led", LEDcontrol);
    messageIN.dispatch("/w", Awink);
   }
 }

void LEDcontrol(OSCMessage &msg, int matched){
     boolean state = LOW;
     char whatLED[] = {'/','1',0};
      for(int i=0; i<6;i++){
        whatLED[1] = char(i | 0x30);
        if(msg.match(whatLED,matched)){
            state = LOW;
            if(msg.getInt(0) > 0) state = HIGH;
            digitalWrite(outPins[i],state);
        } 
  }
}

void Awink(OSCMessage &msg) {
  for(int i=0; i<8; i++){
  wink(300,0);
  }
}

void wink(int del,byte pin){
    digitalWrite(outPins[pin],HIGH);
    delay(del);
    digitalWrite(outPins[pin],LOW);
    delay(del);
}

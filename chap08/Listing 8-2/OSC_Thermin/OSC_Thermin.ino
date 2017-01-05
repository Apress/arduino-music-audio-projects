/*
OSC Theremin - Mike Cook
Controls a PD theremin patch
*/
#include <OSCBundle.h>
#include <OSCBoards.h>
#include <OSCMessage.h>

int lastAnalogueValue[2];
float lastValueSent[2];
float changeThreshold[] = { 0.2, 0.8 };

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
 SLIPEncodedSerial SLIPSerial(Serial);
#endif

void setup() {
  analogReference(EXTERNAL);
  SLIPSerial.begin(38400);   // set this as high as you can reliably run on your platform
#if ARDUINO >= 100
  while(!Serial)
    ; //Leonardo requirement to establish USB device
#endif
}
   
void loop(){
  checkAnalogue(); // look at analog ports 0 & 1
  delay(30); // limit the rate of change
}


void checkAnalogue(){

  char messageAnalog[] ="/a/0";
  int currentAnalogueReading;
  float value_to_send;
  for(int i=0; i<2; i++){
    currentAnalogueReading = 1027 - analogRead(i);
    currentAnalogueReading = 1027 - analogRead(i);
      lastAnalogueValue[i] = currentAnalogueReading;
      messageAnalog[3] = char(i + 0x30);
      OSCMessage msg(messageAnalog);
      if(i == 0){ // conditioning for right hand frequency
        value_to_send = mapfloat((float) currentAnalogueReading, 0.0, 1023.0, 100.0, 2000.0);
      }
      else {  // conditioning for left hand amplitude      
       if(lastAnalogueValue[0] < 870 && lastAnalogueValue[1] < 870){
         value_to_send = mapfloat((float) currentAnalogueReading, 0.0, 870.0, 100.0, 0.0);
       }
      else {
        value_to_send = 0.0;
       }
      }
      msg.add((float)value_to_send);
      // now send the message
      if( abs(value_to_send - lastValueSent[i]) > changeThreshold[i] || ( lastValueSent[1] !=0.0 && value_to_send == 0.0 ) ) {
      SLIPSerial.beginPacket();  
      msg.send(SLIPSerial); // send the bytes to the SLIP stream
      SLIPSerial.endPacket(); // mark the end of the OSC Packet
      msg.empty(); // free space occupied by message
      lastValueSent[i] = value_to_send;
      }
   // }
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


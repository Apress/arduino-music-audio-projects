/* Nunchuck Air Drums - Mike Cook
* plays a different drum depending on the buttons pressed
*/

#include <I2C.h>
 
 int outbuf[6];		// array to store results
 int z_button = 0;
 int c_button = 0;
 long timeBetweenReads = 2; // max speed to read
 long timeOfNextRead=0;
 boolean yTrig = false;
 const byte led = 13;
 byte drums[] = { 56, 49, 40, 35}; // drums to use
 byte lastDrum;

void setup () {
  Serial.begin(31250); // MIDI baud rate
  I2c.begin();
  I2c.pullup(0); // disable I2C pull ups
  I2c.timeOut(500); // half a second to prevent lock up
  I2c.write(0x52, 0x40, 0x00); // send the initialisation handshake
  pinMode(led,OUTPUT);
  digitalWrite(led,LOW);
  lastDrum = drums[0]; // for first turn off
}

void loop () {
  readNunchuck();
  processResults();
}

void readNunchuck () {
 while(millis() < timeOfNextRead) { } // hold if trying to read too fast
 I2c.read(0x52, 6);  // I2c.read(address, numberBytes)
 for(int j=0; j<6; j++){   // now get the bytes one at a time
      outbuf[j] =  0x17 + (0x17 ^ I2c.receive());     // receive a byte
     }  
  I2c.write(0x52, 0x00); // send the request for next bytes
  timeOfNextRead = millis() + timeBetweenReads;
}


void processResults(){
  if( outbuf[3] < 120 && !yTrig) { // arm trigger
      digitalWrite(led,HIGH);
     yTrig = true;
    }
  if( outbuf[3] > 160 && yTrig) { // fire trigger
     digitalWrite(led,LOW);
     noteSend(0x80,lastDrum, 0); // turn off last note
     lastDrum = drums[outbuf[5] & 0x03];
     noteSend(0x90,lastDrum, 127); // hit as hard as you can
     yTrig = false;
   }
}

void noteSend(byte cmd, byte data1, byte data2) {
  cmd = cmd | (byte) 9;  // drums
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

// test function only
void TestProcessResults(){
   if( outbuf[3] < 120 && !yTrig) { // arm trigger
       digitalWrite(led,LOW);
       Serial.println("Armed trigger");
       yTrig = true;
     }
  if( outbuf[3] > 160 && yTrig) { // fire trigger
    digitalWrite(led,LOW);
    Serial.println("Hit");
    Serial.print("X reading ");
    Serial.print(outbuf[2]);
    Serial.print ("\t Y reading ");
    Serial.println(outbuf[4]);
    yTrig = false;
  }
}


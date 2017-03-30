// Nunchuk test
#include <I2C.h>
 
 int outbuf[6];		// array to store results
 int z_button = 0;
 int c_button = 0;
 long timeBetweenReads = 5; // max speed to read
 long timeOfNextRead=0;

void setup () {
  Serial.begin (9600);
  I2c.begin();
  I2c.pullup(0); // disable I2C pull ups
  I2c.timeOut(500); // half a second to prevent lock up
  I2c.write(0x52, 0x40, 0x00); // send the initilization handshake
  Serial.println("Press the Z button to read");
}

void loop () {
  readNunchuck();
  if(z_button == 0)printResults();
}

void readNunchuck () {
 while(millis() < timeOfNextRead) { } // hold if trying to read too fast
 I2c.read(0x52, 6);  // I2c.read(address, numberBytes)
 for(int j=0; j<6; j++){   // now get the bytes one at a time
      outbuf[j] =  0x17 + (0x17 ^ I2c.receive());     // receive a byte
     }  
  I2c.write(0x52, 0x00); // send the request for next bytes
  timeOfNextRead = millis() + timeBetweenReads;
  // process the results
  // X & Y joystick are in outbuffer 0 & 1 already
  z_button = outbuf[5] & 1; 
  c_button = (outbuf[5] >> 1) & 1;
  outbuf[2] = (outbuf[2] << 2) | (outbuf[5] >> 2) & 0x3; // acc x
  outbuf[3] = (outbuf[3] << 2) | (outbuf[5] >> 4) & 0x3; // acc y
  outbuf[4] = (outbuf[4] << 2) | (outbuf[5] >> 6) & 0x3; // acc z
}

void printResults (){
 for(int i=0; i<5; i++){
  Serial.print (outbuf[i], DEC);
  Serial.print ("\t");
 }
  Serial.print (z_button, DEC);
  Serial.print ("\t");
  Serial.print (c_button, DEC);
  Serial.print ("\t");
  Serial.println(" ");
}


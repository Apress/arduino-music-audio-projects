// Nunchuck test
// By Mike Cook
#include <I2C.h>
 
int outbuf[6];		// array to store results
void setup () {
  Serial.begin (19200);
  I2c.begin();
  I2c.pullup(0); // disable I2C pull ups
  I2c.timeOut(500); // half a second to prevent lock up
  Serial.print ("Finished setup\n");
  nunchuck_init (); // send the initialisation handshake
}

void nunchuck_init () {
 // I2c.write(address, registerAddress, data)  // set up start of read
  I2c.write(0x52, 0x40, 0x00);
}

void send_zero () { 
  I2c.write(0x52, 0x00);
}

void loop () {
 // I2c.read(address, numberBytes)
 Serial.println(" ");
 Serial.println("Raw data");
 I2c.read(0x52, 6);
 for(int j=0; j<6; j++){   // now get the bytes one at a time
      outbuf[j] =  I2c.receive();     // receive a byte
      if(outbuf[j] < 0x10) Serial.print("0"); // print leading zero if needed
      Serial.print(outbuf[j], HEX);          // print the byte
      Serial.print(" ");         
     }
     Serial.println(" ");  // new line
   printResults();  
  send_zero (); // send the request for next bytes
  delay (800);
}

// Print the input data we have received
// accel data is 10 bits long
// so we read 8 bits and the LS bits are in the last byte
void
printResults ()
{
  int z_button = 0;
  int c_button = 0;

 // byte outbuf[5] contains bits for z and c buttons
 // it also contains the least significant bits for the accelerometer data
 // so we have to check each bit of byte outbuf[5]
 z_button = outbuf[5] & 1;
 c_button = 1 ^ ((outbuf[5] >> 1) & 1) ^ (outbuf[5] & 1);
 
 outbuf[2] = (outbuf[2] << 2) | (outbuf[5] >> 2) & 0x3; // acc x
 outbuf[3] = (outbuf[3] << 2) | (outbuf[5] >> 4) & 0x3; // acc y
 outbuf[4] = (outbuf[4] << 2) | (outbuf[5] >> 6) & 0x3; // acc x
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


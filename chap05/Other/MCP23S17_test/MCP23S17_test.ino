/* Routines for accessing MCP23S17 SPI Port expander - Mike Cook
  * CS - to digital pin 10  (SS pin)
  * SDI - to digital pin 11 (MOSI pin)
  * CLK - to digital pin 13 (SCK pin)
*/
const int slaveSelectPin = 10;
const byte expander0_write = 0x40, expander0_read = 0x41; 
const byte expander1_write = 0x42, expander1_read = 0x43; 

#include <Spi.h>
void setup(){
    // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin, OUTPUT);
  // initialise SPI:
  SPI.begin();
  expanderW(expander0_write, 0x0A, SEQOP | HAEN); // this sets both / all expanders to use address pins
  expanderW(expander0_write, 0x0A, SEQOP | HAEN); // this sets both / all expanders to use address pins        
  expanderW(expander0_write,IODIRA, 0xff);      // Data direction register A all inputs
  expanderW(expander0_write,IODIRB, 0xff);      // Data direction register B all inputs
  expanderW(expander0_write,IPOLA, 0xff);       // Input polarity read an earth (press) as a one
  expanderW(expander0_write,IPOLB, 0xff);       // Input polarity read an earth (press) as a one
  expanderW(expander0_write,INTCONA, 0x00);     // Notify on change
  expanderW(expander0_write,INTCONB, 0x00);     // Notify on change
  expanderW(expander0_write,GPINTENA, 0xff);    // enable notifacation on pins
  expanderW(expander0_write,GPINTENB, 0xff);    // enable notifacation on pins

  expanderW(expander1_write, 0x0A, SEQOP | HAEN); // this sets both / all expanders to use address pins
  expanderW(expander1_write, 0x0A, SEQOP | HAEN); // this sets both / all expanders to use address pins        
  expanderW(expander1_write,IODIRA, 0xff);      // Data direction register A all inputs
  expanderW(expander1_write,IODIRB, 0xff);      // Data direction register B all inputs
  expanderW(expander1_write,IPOLA, 0xff);       // Input polarity read an earth (press) as a one
  expanderW(expander1_write,IPOLB, 0xff);       // Input polarity read an earth (press) as a one
  expanderW(expander1_write,INTCONA, 0x00);     // Notify on change
  expanderW(expander1_write,INTCONB, 0x00);     // Notify on change
  expanderW(expander1_write,GPINTENA, 0xff);    // enable notifacation on pins
  expanderW(expander1_write,GPINTENB, 0xff);    // enable notifacation on pins
}

void loop(){
}

byte expanderR(byte deviceOpcode,byte regAddress){ // expander read
  byte value;
  digitalWrite(slaveSelectPin, LOW);
  Spi.transfer(deviceOpcode);  // address read
  Spi.transfer(regAddress);   //  register address
  value = Spi.transfer(0x0);   //  dummy data for read
  digitalWrite(slaveSelectPin, HIGH);
  return value;
}

byte expanderW(byte deviceOpcode, byte regAddress, byte data){ // expander write
  digitalWrite(slaveSelectPin, LOW);
  Spi.transfer(deviceOpcode);  // address write
  Spi.transfer(regAddress);   //  register address
  Spi.transfer(data);   //  register data
  digitalWrite(slaveSelectPin, HIGH);
}


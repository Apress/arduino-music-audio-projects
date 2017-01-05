// hash defines
// Port expander registers
#define IODIR (byte) 0x00
#define IPOL (byte) 0x02
#define GPINTEN (byte) 0x04
#define DEFVAL (byte) 0x06
#define INTCON (byte) 0x08
#define IOCON (byte) 0x0A
#define GPPU (byte) 0x0C
#define INTF (byte) 0x0E
#define INTCAP (byte) 0x10
#define GPIO (byte) 0x12
#define OLAT (byte) 0x14

// Bits in the IOCON register
#define BANK (byte) 0x80
#define MIRROR (byte) 0x40
#define SEQOP (byte) 0x20
#define DISSLW (byte) 0x10
#define HAEN (byte) 0x08
#define ODR (byte) 0x04
#define INTPOL (byte) 0x02

// I2C device addresses
#define chordSwitchAddress   0x20  // address of chord switches
#define chordLEDaddress   0x21  // address of chord red led indicator
#define stringSwitchAddress  0x22  // address of string switches
#define controlSwitchaddress 0x24  // address of control switches & indicators
// Control Switch device bit masks
#define R1S 0x01                 // rotary encoder 1 step
#define R2S 0x02                // rotary encoder 2 step
#define playSwitch 0x04        // play
#define auxSwitch 0x08        // Aux
#define octave1Switch 0x10         // Octiave 1
#define octave2Switch 0x20        // Octiave 2
#define pick1Switch 0x40        // Pick 1
#define pick2Switch 0x80       // Pick 2
#define pushB  0x0100         // Push button
#define capoLEDr 0x0200      // Mode 1 LED red
#define pickingLEDr 0x0400  // Mode 2 LED red
#define auxLEDr 0x0800     // Aux 3 LED red
#define capoLEDg 0x1000      // Mode 1 LED green
#define pickingLEDg 0x01    // Mode 2 LED green
#define auxLEDg 0x02       // Aux LED green
#define pickingSpeedLEDr 0x2000   // RGB LED 2 red
#define pickingSpeedLEDg 0x4000   // RGB LED 2 green
#define pickingSpeedLEDb 0x8000   // RGB LED 2 blue

// String Switch device bit masks
#define finger1 0x20        // Finger 1 touch switch
#define finger2 0x04        // Finger 2 touch switch
#define finger3 0x08        // Finger 3 touch switch
#define finger4 0x10        // Finger 4 touch switch
#define ddrString 0x03c     // data direction register for string switch
#define ddrChord 0x03ff     // data direction register for chord switch
#define ddrControl 0x01ff         // data direction register for control switch

// Expander interrupt pins
#define stringChange 2   // string change button
#define thumbIn 3        // Thumb optical switch
#define chordChange 4    // chord change button
#define controlChange 5  // control I2C indicator
// Rotary encoder
#define R1D 7       // rotary encoder 1 direction
#define R2D 8       // rotary encoder 2 direction
// chord display LED
#define chordShowLEDr 9    // RGB LED red (PWM)
#define chordShowLEDg 10   // RGB LED green (PWM)
#define chordShowLEDb 11   // RGB LED blue (PWM)
// other
#define TRUE HIGH
#define FALSE LOW



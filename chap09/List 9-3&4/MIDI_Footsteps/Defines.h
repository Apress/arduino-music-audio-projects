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
#define ledAddress   (0x20 | 0x0)  // address of trigger LED indicators output
#define ddrTrigger 0x00000     // data direction register for trigger indictor LEDs

#define midiChannel (byte)0

// Arduino pin assignments
const int s0Pin = 2;  // multiplexer select pins
const int s1Pin = 3;
const int s2Pin = 4;
const int mux1 = 0;   // analogue port multiplexer 1 is read on
const int mux2 = 1;   // analogue port multiplexer 2 is read on
const int notesSelect = 5; // MIDI notes or CC

// Variable definitions
int currentState[16]; // current state of sensors
int lastState[16];    // the last state of the sensors
int threshold = 0x90;  // sets the threshold value in deciding if a sensor is pressed.
int lastLedVal;



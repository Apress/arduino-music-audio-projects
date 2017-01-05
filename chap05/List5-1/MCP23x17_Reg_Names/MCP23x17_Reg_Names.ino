// MCP23S17 addresses in IOCON.BANK = 0
#define IODIRA (byte) 0x00
#define IODIRB (byte) 0x01
#define IPOLA (byte) 0x02
#define IPOLB (byte) 0x03
#define GPINTENA (byte) 0x04
#define GPINTENB (byte) 0x05
#define DEFVALA (byte) 0x06
#define DEFVALB (byte) 0x07
#define INTCONA (byte) 0x08
#define INTCONB (byte) 0x09
#define IOCON (byte) 0x0A
#define GPPUBA (byte) 0x0C
#define GPPUBB (byte) 0x0d
#define INTFA (byte) 0x0E
#define INTFB (byte) 0x0F
#define INTCAPA (byte) 0x10
#define INTCAPB (byte) 0x11
#define GPIOA (byte) 0x12
#define GPIOB (byte) 0x13
#define OLATA (byte) 0x14
#define OLATB (byte) 0x15
// Bits in the IOCON register
#define BANK (byte) 0x80
#define MIRROR (byte) 0x40
#define SEQOP (byte) 0x20
#define DISSLW (byte) 0x10
#define HAEN (byte) 0x08
#define ODR (byte) 0x04
#define INTPOL (byte) 0x02


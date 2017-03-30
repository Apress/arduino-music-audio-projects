// Arduino pin assignments
#define midiChannel (byte)0

// Define constants
#define a3 45
#define b3 47
#define c3 48 
#define d3 50 
#define e3 52 
#define f3 53 
#define g3 55
#define a4 57 
#define b4 59 
#define c4 60   // Middle C (MIDI note value 60) 
#define d4 62
#define e4 64
#define f4 65
#define g4 67
#define a5 69
#define b5 71

// Variable definations
int redLED = 9, greenLED = 10, blueLED = 11;  // PWM lnes for RGB LED
int bankColourR[8]    =  { 255, 0,  64, 128,  197, 128,   255, 0 }; // red colour for bank
int bankColourG[8]    =  { 0, 255,  128, 255,  128, 210,  255, 64 }; // green colour for bank
int bankColourB[8]    =  { 255,255,  255, 197,  255, 128,  64, 32 }; // blue colour for bank
int bankchangeSensor = 2;  // sensor for changing banks
int bank = 0;       // bank change
const int maxSensors = 15;   // number of sensors being used
int sensorLast[maxSensors];    // the last state of the sensors
int sensor[] = {2,3,4,5,6,7,8,12,13,14,15,16,17,18,19};  // sensor pins
char notePlaying[16];
char note[16][4] = {a3, b3, c3, d3, e3, f3, g3, a4, b4, c4, d4, e4, f4, g4, a5, b5,
                   b4, c4, d4, e4, f4, g4, a5, b5, a3, b3, c3, d3, e3, f3, g3, a4,
                   e3, f3, g3, a4, a3, b3, c3, d3, f4, g4, a5, b5,  b4, c4, d4, e4,
                   b5, a5, g4, f4, e4, d4, c4, b4, a4, g3, f3, e3, d3, c3, b3, a3
                };    // The MIDI note value to be played



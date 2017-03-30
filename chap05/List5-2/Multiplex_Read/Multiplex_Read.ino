/* Multiplexed analogue inputs - Mike Cook
* For two 4051 multiplexers parallel wiring
* 16 inputs with threshold control
*/

// Define constants
const int s0Pin = 2;  // multiplexer select pins
const int s1Pin = 3;
const int s2Pin = 4;
const int mux1 = 0;   // analogue port multiplexer 1 is read on
const int mux2 = 1;   // analogue port multiplexer 2 is read on
const byte channel = 0; // MIDI channel

// Variable definitions
int currentState[16]; // current state of sensors
int lastState[16];    // the last state of the sensors
int threshold = 0x4;  // sets the threshold value in deciding if a sensor has changed
// MIDI controller channels to send control data out on
char control[16] = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

// Start of code
void setup() {
  //  set the states of the I/O pins to drive the sensor multiplexer:
   pinMode(s0Pin, OUTPUT);
   pinMode(s1Pin, OUTPUT);
   pinMode(s2Pin, OUTPUT);
   Serial.begin(31250); //MIDI baud rate
}

//********************* MAIN LOOP ***********************************

void loop() {
    doSensorScan();
    lookForChange();
    saveCurrentState();
    } // end loop function
    
//********************* Functions *********************************** 

void doSensorScan() {  // look at all the sensors
 for(int i=0; i<8; i++){
   // select multiplexer channel
   digitalWrite(s0Pin, i & 0x1);
   digitalWrite(s1Pin, (i>>1) & 0x1);
   digitalWrite(s2Pin, (i>>2) & 0x1);
   currentState[i] = analogRead(mux1); // dummy read to allow sample & hold capacitor to charge
   currentState[i] = analogRead(mux1); // read mux1 in first 8 array locations
   currentState[i+8] = analogRead(mux2); // dummy read to allow sample & hold capacitor to charge
   currentState[i+8] = analogRead(mux2); // read mux2 in last 8 array locations
 }  
}

void saveCurrentState(){  // save the current state for comparison next time
  for(int i=0; i<16; i++){
    lastState[i] = currentState[i];
  }
}

// the value of threshold determines if we send a new message
void lookForChange(){
  for(int i=0; i<16; i++){
    if(abs(currentState[i] - lastState[i] ) > threshold) { // if we have a sufficient change
      controlSend(channel, control[i], currentState[i]>>3);  // send control change message
    }
  }
}
// use this function if you want to send MSB and LSB CC messages
void lookForChange2(){
  for(int i=0; i<16; i++){
    if(abs(currentState[i] - lastState[i] ) > threshold) { // if we have a sufficient change
      controlSend(channel, control[i], currentState[i]>>3);  // send MSB
      controlSend(channel, control[i]+32, (currentState[i] & 0x7) << 4);  // send LSB
    }
  }
}

 void controlSend(byte channel, byte controller, byte value) {
  byte cmd = 0xB0 | channel;  // merge channel number
  Serial.write(cmd);
  Serial.write(controller);
  Serial.write(value);
}


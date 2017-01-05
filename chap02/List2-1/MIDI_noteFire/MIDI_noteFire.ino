
/* Midi note fire - Mike Cook
 *
 * send MIDI serial data, automatically for a test
 * 
*/
#define midiChannel (byte)0 // Channel 1

// Start of code
void setup() {
 //  Setup serial
   Serial.begin(9600);    // Debug speed
}

void loop() {
  int val;
  val = random(20,100);
    noteSend(0x90, val, 127);
    delay(200);
    noteSend(0x80, val, 127);
   delay(800);
    } // end loop function

//  plays a MIDI note
 void noteSend(byte cmd, byte data1, byte data2) {
  cmd = cmd | midiChannel;  // merge channel number
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}


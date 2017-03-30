
/* Midi note fire for the Due - Mike Cook
 * using serial 1
 * send MIDI serial data, automatically for a test
 * 
*/
#define midiChannel (byte)0 // Channel 1

// Start of code
void setup() {
 //  Setup serial1
   Serial1.begin(31250);    // MIDI speed
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
  Serial1.write(cmd);
  Serial1.write(data1);
  Serial1.write(data2);
}


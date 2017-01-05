// Variables setup 

// The MIDI note value to be played (open strings e3 a3 d4 g4 b4 e5 )
char   note[6] = {52, 57, 62, 67, 71, 76};  
int  lastNotePlayed[6] = {0,0,0,0,0,0};     // note turned on when you press the switch
int  notePlaying[6] = {0,0,0,0,0,0};        // note playing on each string 0 for not sounding

byte pattern[12][12] =   {
                          0,1,2,3,4,5,6,6,6,6,6,6,       // pattern for strum 
                          5,4,3,2,1,0,6,6,6,6,6,6,       // strings 0 to 5 pattern end in a 6
                          0,1,2,3,4,5,4,3,2,1,0,6,
                          5,4,3,2,1,0,1,2,3,4,5,6,
                          
                          1,2,4,3,1,2,4,3,6,6,6,6,        // bank 2
                          2,3,5,4,2,5,3,4,6,6,6,6,
                          1,4,5,2,4,0,5,2,4,6,6,6,
                          0,4,2,5,0,4,2,5,6,6,6,6,
                          
                          0,5,1,4,2,3,4,2,1,5,0,6,         // bank 3
                          0,2,4,1,3,5,6,6,6,6,6,6,
                          0,1,2,3,0,1,2,3,4,5,6,6,
                          0,1,2,3,4,5,3,4,5,6,6,6
                         };

byte patternDelay[12][12] = {
                             2,2,2,2,2,2,2,1,1,1,1,1,       // delay between the notes
                             2,2,2,2,2,2,2,1,1,1,1,1,
                             1,1,1,1,1,1,1,1,1,1,1,1,
                             1,1,1,1,1,1,1,1,1,1,1,1,
                          
                             1,1,1,1,1,1,1,1,1,1,1,1,      // bank 2
                             1,0,1,2,1,1,1,1,1,1,1,1,
                             0,0,0,1,1,2,1,1,2,1,1,1,
                             1,1,1,1,1,1,1,1,1,1,1,1,
                           
                             0,2,0,2,0,2,0,2,0,2,1,1,    // bank 3
                             2,2,2,2,2,2,1,1,1,1,1,1,
                             1,1,1,1,1,1,1,1,1,1,1,1,
                             1,1,1,1,1,1,1,1,1,1,1,1
                           };

int chordColourR[8]    =  { 0, 255, 196, 128,  64, 128,   0, 255 }; // red colour for chord
int chordColourG[8]    =  { 0,   0, 128,   0, 128,  32,   0, 196 }; // green colour for chord
int chordColourB[8]    =  { 0,   0,   0,  64,   0, 128, 196, 230 }; // blue colour for chord

// Time variables
int metronome = 0;
long pickTime;
int del,thumbNow = 1, thumbLast = 1; 

int currentTriggerState, lastChord = 0, currentChord, lastControl = 0, lastString = 0;
// for bit manipulation of LEDs
int redChordLED = 0, greenChordLED = 0, stringLED = 0, modeLED = auxLEDr; 
int pickPat = 0;  // picking pattern
int patShift = 0; // shift on the picking pattern

// Playing
byte midiChannel = 0;  // channel to use
boolean patPlaying = false, playing = true;
int playString = 0;   // string to play in picking pattern
int picksInPat = 10;  // Number of pick in this pattern
int pickNum = 0;      // How far in the pattern you have gone
int delCount = 0, lastDel =0 ;     // delay count down for spacing out time between picks
int pat = 0;         // picking pattern to use
int voice=0;        // index into the voice number to send
boolean push = false; // push button variable
int capo = 0;         // capo adding number
int octave = 0;       // octave value derived from switches

// switch states
boolean pick2SwitchState=false, pick1SwitchState=false, octave1SwitchState=false;
boolean octave2SwitchState=false;
int rot1 = 0x40, rot2 = 100;
int rot2PlayStore = 100, rot2StoreVoice = 0;


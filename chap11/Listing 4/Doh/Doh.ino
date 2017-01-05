// Do-Re-Mi from the Sound of music
#define C_0 0
#define D_0 1
#define E_0 2
#define F_0 3
#define G_0 4
#define A_0 5
#define B_0 6
#define C_1 7
 int pitch[] = {1911, 1702, 1516, 1431, 1275, 1136, 1012, 955};   
byte melody[] = { C_0, 3, D_0, 1, E_0, 3, C_0, 1, E_0, 2, C_0, 2, E_0, 4, 
                  D_0, 3, E_0, 1, F_0, 1, F_0, 1, E_0, 1, D_0, 1, F_0, 8, 
                  E_0, 3, F_0, 1, G_0, 3, E_0, 1, G_0, 2, E_0, 2, G_0, 4, 
                  F_0, 3, G_0, 1, A_0, 1, A_0, 1, G_0, 1, F_0, 1, A_0, 8, 
                  G_0, 3, C_0, 1, D_0, 1, E_0, 1, F_0, 1, G_0, 1, A_0, 8, 
                  A_0, 3, D_0, 1, E_0, 1, F_0, 1, G_0, 1, A_0, 1, B_0, 8, 
                  B_0, 3, E_0, 1, F_0, 1, G_0, 1, A_0, 1, B_0, 1, C_1, 6, 
                  B_0, 1, B_0, 1, A_0, 2, F_0, 2, B_0, 2, G_0, 2, C_1, 2,
                  G_0, 2, E_0, 2, D_0, 2, 
// second verse
                  C_0, 3, D_0, 1, E_0, 3, C_0, 1, E_0, 2, C_0, 2, E_0, 4, 
                  D_0, 3, E_0, 1, F_0, 1, F_0, 1, E_0, 1, D_0, 1, F_0, 8, 
                  E_0, 3, F_0, 1, G_0, 3, E_0, 1, G_0, 2, E_0, 2, G_0, 4, 
                  F_0, 3, G_0, 1, A_0, 1, A_0, 1, G_0, 1, F_0, 1, A_0, 8, 
                  G_0, 3, C_0, 1, D_0, 1, E_0, 1, F_0, 1, G_0, 1, A_0, 8, 
                  A_0, 3, D_0, 1, E_0, 1, F_0, 1, G_0, 1, A_0, 1, B_0, 8, 
                  B_0, 3, E_0, 1, F_0, 1, G_0, 1, A_0, 1, B_0, 1, C_1, 6, 
                  B_0, 1, B_0, 1, A_0, 2, F_0, 2, B_0, 2, G_0, 2, C_1, 10, 
                  C_0, 1, D_0, 1, E_0, 1, F_0, 1, G_0, 1, A_0, 1, B_0, 1, 
                  C_1, 2, G_0, 2, C_0, 2, 
                  -1, -1 };

const byte soundPin = 13;
int songTempo = 200;
void setup() {
  pinMode(soundPin,OUTPUT);
}

void loop() {
  int place = 0;
  while(melody[place] != (byte)-1){
     note(pitch[melody[place]], melody[place+1]*songTempo);
     delay(60);
     place +=2;
   }
  delay(4000); // pause before repeat
}

void note(int halfPeriod, long interval) {
  long startTime = millis(); 
  while(millis()-startTime < interval) {
     digitalWrite(soundPin,HIGH);
     delayMicroseconds(halfPeriod);
     digitalWrite(soundPin,LOW);
     delayMicroseconds(halfPeriod);
  } 
}

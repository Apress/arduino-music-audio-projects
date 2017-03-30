// Notch filter - Mike Cook
// using int buffers
// Open up the plot window
const int bufferSize = 250;
int inBuffer[bufferSize];
int outBuffer[bufferSize];
void setup() {
Serial.begin(250000);
displayWave(); // clear display
makeWave(0); // create input buffer wave 
notchFilter(); // do the filtering
displayWave(); // to the plot window
}

void loop() {
}

void makeWave(int wave){
  // clear buffers
  for(int i=0; i<bufferSize; i++){
    outBuffer[i] = 0;
     }
    for(int i=0; i<bufferSize; i++){
    inBuffer[i] = 0;
     }
switch(wave){
 case 0:
   makeTriangle();  
   makeSin();
   break;
 case 1:
   makeSquare();
 case 2:
   inBuffer[0] = 120; // inpulse  
   }
}

void makeSin(){
  // increment controls the frequency
  int count = 0;
  int increment= 18;
  for(int i=0; i<bufferSize; i++){
   count += increment; 
   if(count > 360) count -= 360;
   inBuffer[i] += (int)(64.0 * sin((float)count / 57.2957795 ));
  }
}

void makeTriangle(){
  // increment/bufferSize determins the number of cycles
    int increment = 6, wave = 100;
  // make a triangle wave
  for(int i=0; i<bufferSize; i++){
       wave += increment;
    if(wave > (120) || wave < -120) {
      increment = -increment;
      wave += increment;
    }
    inBuffer[i] = wave;
  }
}

void makeSquare(){
  int count = 0, limit = 10;;
  boolean wave = true;
  for(int i=0; i<bufferSize; i++){
  if(wave) inBuffer[i] = 120; else inBuffer[i] = -120;
  count++;
  if(count >= limit){
    count = 0;
    wave = !wave;
  }
   }
}

void notchFilter(){ // for a 20 sample wave
  // prime output buffer
   outBuffer[0] = 0; // inBuffer[0]
   outBuffer[1] = 0; // inBuffer[1]
   for(int i=2; i<(bufferSize); i++){
    outBuffer[i] = ((1.8523*(float)outBuffer[i-1] - 0.94833* (float) outBuffer[i-2] 
       + (float)inBuffer[i] - 1.9021*(float) inBuffer[i-1] + (float)inBuffer[i-2]));
   }
}

void displayWave(){
  for(int i=0; i<bufferSize; i++){
    Serial.println(inBuffer[i]);
  }
  for(int i=0; i<bufferSize; i++){
    Serial.println(outBuffer[i]);
  }
}


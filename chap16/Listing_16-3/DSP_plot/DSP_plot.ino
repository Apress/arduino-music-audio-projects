// Dsp filter plot - Mike Cook
// plots the frequency response of 7 filters
// Open up the plot window
const int bufferSize = 250;
int inBuffer[bufferSize];
int outBuffer[bufferSize];
int displayCount = 499;
float att =0.0;

void setup() { 
 int steps;
 Serial.begin(250000);
  for(int k=0; k<7; k++){
    clearPlot();
    displayCount = 499;
    if(k < 3) steps = 60; else steps = 100;
    for(int i=2; i<steps; i++){
       makeWave(i); // create input buffer wave 
       filter(k); // do the filtering 
       measure(); //
    } 
  while(displayCount > 0){ // shift to end
    Serial.println(att);
    displayCount--; 
    }
  delay(2000);
  }
}

void loop() { // do nothing
}

void makeWave(int freq){
  float count = 0.0;
  float increment= 360.0 / ((float)(bufferSize+1) / (float)freq);
  for(int i=0; i<bufferSize; i++){
   inBuffer[i] = (int)(4960.0 * sin(count / 57.2957795 ));
   outBuffer[i] = 0;
      count += increment; 
   if(count > 360.0) count -= 360.0;
  }
}

void measure(){ 
  int point = bufferSize - 1;
  float accIn = 0.0, accOut = 0.0;
   while(point > 0){
     if(outBuffer[point] > 0) accOut += (float)outBuffer[point];
      else
       accOut -= (float)outBuffer[point];
     if(inBuffer[point] > 0) accIn += (float)inBuffer[point];
     else
       accIn -= (float)inBuffer[point];
     point --;
   }
   att = 10* log(accOut / accIn);
   for(int i=0; i<4; i++) { 
    Serial.println(att);
    displayCount--;
  } 
}

void filter(int f){
  switch(f){
    case 0: average(2);
    break;
    case 1: average(3);
    break;
    case 2: average(4);
    break;
    case 3: notchFilter();
    break;
    case 4: bandPass();
    break;
    case 5: highPass(1);
    break;
    case 6: highPass(-1);  
  }
}

void highPass(int n){
   for(int i=1; i<(bufferSize); i++){
       outBuffer[i] = 0.3*(float)outBuffer[i-1] +(float)n*0.3*(float)inBuffer[i] - 0.3*(float)inBuffer[i-1];               
   }
}

void bandPass(){
   for(int i=2; i<(bufferSize); i++){
    outBuffer[i] = (0.9*(float)outBuffer[i-1] - 0.8* (float) outBuffer[i-2] 
       + (float)inBuffer[i] );
   }
}

void notchFilter(){ // for a 20 sample wave
   for(int i=2; i<(bufferSize); i++){
      outBuffer[i] = ((1.8523*(float)outBuffer[i-1] - 0.94833* (float) outBuffer[i-2] 
       + (float)inBuffer[i] - 1.9021*(float) inBuffer[i-1] + (float)inBuffer[i-2]));
   }
}

void average(int n){
  int acc;
   for(int i=0; i<(bufferSize-n); i++){
    acc=0;
    for(int j=0; j<n; j++){
      acc += inBuffer[i+j];
    }
    outBuffer[i]= acc/n; // save average
   }
}

void clearPlot(){
    for(int i=0; i<500; i++){
    Serial.println(0);
  }
}


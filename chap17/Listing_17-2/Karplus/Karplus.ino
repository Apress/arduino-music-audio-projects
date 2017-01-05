// Karplus viewer - Mike Cook
// using int buffers
// Open up the plot window

const int bufferSize = 500;
int outBuffer[bufferSize];
void setup() {
Serial.begin(250000);
primeBuffer(); 
delay(2000);
pinMode(13,OUTPUT);
}

void loop() {
  for(int i = 0; i < bufferSize; i++){
    karplus();
  }
}

void primeBuffer(){
  // clear buffers
  int displayCount = 500;
  for(int i=0; i<480; i++){
    outBuffer[i] = random(16000) - 8000;
    Serial.println(outBuffer[i]);
    displayCount --;
   }
   while(displayCount> 0) {
    Serial.println(0);
    displayCount --;
   }
 }
 
void karplus(){
static int inPoint = bufferSize -1;
static int outPoint1 = 0;
static int outPoint2 = 1;

    outBuffer[inPoint] = (outBuffer[outPoint1] + outBuffer[outPoint2]) >> 1;
    Serial.println(outBuffer[inPoint]);
    // move pointers
    inPoint++;
    if(inPoint >= bufferSize -1 ) inPoint = 0;
        outPoint1++;
    if(outPoint1 >= bufferSize -1) outPoint1 = 0;
    outPoint2++;
    if(outPoint2 >= bufferSize -1) outPoint2 = 0;
    Serial.flush(); // wait until buffer empties - gives a smoother graph

}


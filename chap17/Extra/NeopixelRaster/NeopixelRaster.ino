// Neopixel serpentine Raster test
// By Mike Cook December 2015

#include <Adafruit_NeoPixel.h>
#define PIN   3 // pin connected to the NeoPixels
#define NUMPIXELS   240 // number of LEDs 240 LEDs in 4 meters
#define xMax 20 // maximum number of coloums
#define yMax 12 // maximum number of rows

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int angleInc = 360 / (xMax / 2);
int partAngleInc = angleInc / yMax;

void setup() {
   pixels.begin(); // This initializes the NeoPixel library.
   wipe();
   Serial.begin(250000);
   Serial.println(partAngleInc);
}

void loop() {
  int pos, stripColour;
  static int fInc = 1;
  static int fillSize = 0;
  int colourAngle = 0;
  delay(10);
  wipe();
  fillSize += fInc;
  if(fillSize >= yMax) {
    fInc = fInc * -1;
  }
  if(fillSize < 0) {
    fInc = fInc * -1;
  }
  for(int x=0; x < xMax; x++){
        setStrip(x,fillSize);
    }
    pixels.show();
  }
    
void setStrip(int strip, int height){
  int stripColour;
  int angle = angleInc * strip;
  if(height > yMax) height = yMax;
  for(int y=0; y < height; y++){
      stripColour = colorH(angle + (y * partAngleInc) );
      pixels.setPixelColor(getPixPos(strip,y) , stripColour);
  }
}
  
int getPixPos(int x, int y){ // for a serpentine raster
   int pos;
   if(x &0x1) {
      pos = x * yMax + (yMax -1 - y) ;
   } else {
   pos = x * yMax + y;
   } 
   return pos;
}

inline int colour( int a, int b, int c){
  return ((a << 16) | (b << 8) | c);      
}

int colorH(int angle){ //# color returned H=angle, S=1, V=1
    // # get angle in range 0 to 255
    while (angle <0 ) angle += 256;   
    while (angle > 255) angle -=256;          
    if (angle < 85) return colour(255 - angle*3, angle*3, 0);        
    if (angle < 170){
        angle -= 85;
        return colour(0, 255 - angle * 3, angle * 3);
    }
    angle -= 170;
    return colour(angle * 3, 0, 255 - angle * 3);
}

void wipe(){
    for(int i=0;i<NUMPIXELS;i++){
       pixels.setPixelColor(i, pixels.Color(0,0,0)); 
       }
}


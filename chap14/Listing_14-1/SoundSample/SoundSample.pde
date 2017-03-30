/**
 * Sound Sample
 * by Mike Cook
 *  Converts 16 bit .wav file
 *  into 8 bit .h file for Arduino sample player
 */
PrintWriter output;
String waveName="yes";
String loadPath="blank";
String savePath="blank";

void loadFile(){   // load waveform definition from disc
   selectInput("Choose wav file","doLoadFile");  // Opens file chooser
 }
 
 void doLoadFile(File selection){
      if (selection == null) {
    println("No file was selected...");
  } else {
    loadPath = selection.getAbsolutePath();
    println(loadPath);
    saveFile(); // place to save results
   }
 }
 
 void saveFile(){   // load waveform definition from disc
   selectOutput("Save converted file","doSaveFile");  // Opens file chooser
 }
 
void doSaveFile(File selection){
      if (selection == null) {
    println("No file was selected...");
  } else {
    savePath = selection.getAbsolutePath();
      waveName = savePath;
     // nibble away at the path name until we just have the file name
     while(waveName.indexOf('/') != -1) { 
        waveName = waveName.substring(1, waveName.length() ); 
      }
    println("wave name "+waveName);
    convertWave();
 }
}

void setup()
{
  loadFile();
}

void convertWave(){
  int lsb, msb, a;
  int max=0, min =0;
  float scale;
byte sample[] = loadBytes(loadPath);
if(sample[8] != 87 || sample[9] != 65 ||sample[10] != 86 ||sample[11] != 69){
    println("Error - "+loadPath+" is not a .wav file");
    exit();
 }
// Print each value, from 0 to 255 
for (int i = 44; i < sample.length; i +=2) { 
  // bytes are from -128 to 127, this converts to 0 to 255 
  lsb = sample[i] & 0xff;
  msb = sample[i+1] & 0xff;
  a = lsb | (msb << 8);
  if( (a & 0x8000) != 0) a= a | 0xFFFF0000; // sign extend
  if( a > max) max = a;
  if( a < min) min = a;
 } 
 
println("max = " + max +" min = "+min);
scale = 255.0/((float)max - (float)min);
println("scale = "+ scale);
int dataLength = (sample.length -40) / 2;
println("data length = 0x"+ hex(dataLength)+" or "+dataLength+" bytes");
msb = (dataLength >> 8) & 0xff;
lsb = dataLength & 0xff;

// output the sample file
output = createWriter(savePath+".h"); 
output.print("const PROGMEM byte "+waveName+"[] = { 0x" + hex((byte)lsb) + ", 0x" + hex((byte)msb) + ", ");
for (int i = 44; i < sample.length; i +=2) { 
    // Every tenth number, start a new line 
  if (((i+8) % 20) == 0) { 
    output.println(); 
  } 
  // bytes are from -128 to 127, this converts to 0 to 255 
  lsb = sample[i] & 0xff;
  msb = sample[i+1] & 0xff;
  a = (lsb | (msb << 8));
  if( (a & 0x8000) != 0) a= a | 0xFFFF0000;
  a = int((float)a * scale) + 128;
  if(a>255) a = 255;
  if(a<0) a = 0;
  output.print("0x"+hex(a).substring(6,8));
  if(i < sample.length - 2) output.print(", ");
   }
  // finish it off
  output.print(" };");
  output.flush(); // Write the remaining data
  output.close(); // Finish the file
  println("saved at "+savePath+".h");
  exit();
}

void draw(){  }


 
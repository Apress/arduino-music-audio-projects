/**
 * TouchOSC Additave Synth input 1
 * 
 * For an additave synth waveforme definer
 * 16 steps per partial
 * By Mike Cook November 2015
 * Requires Processing 3
 * And an iPad running TouchOSC to change the sliders
 * If you have trouble connecting then create an adhock network and join that
 */

import processing.serial.*;
import controlP5.*;
import oscP5.*;
import netP5.*;
OscP5 oscP5;
NetAddress myRemoteLocation;
ControlP5 synth;
Textlabel loadLab, saveLab, fund, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11;

PrintWriter output;
Serial port; // Create object from Serial class
String iPadIP = "192.168.1.82"; // *** change this to the address of your iPad / iPhone ***
String adaptor = "/dev/cu.SLAB_USBtoUART";  // ***the name of the device driver to use ***
String[] lines; // for input of a file
String waveName="Wave Name";
String savePath;
boolean [] buttonPressed = new boolean [2];
boolean [] multiFadeNeedsRedraw = new boolean [11];
boolean mouseHit = false, loadCallback = false, saveCallback = false;
float [][] multiFade = new float [11][17];
float incSin = TWO_PI/256.0;  // conversion for look up table x axis
int [] [] waveform = new int [16] [256];
int faderHight = 90;
int faderWidth = 245;
int buttonNumber = -1;

void setup() {
  size(850,790);
  frameRate(30);
  background(0);
  colorMode(RGB, 255);
  for(int i =0; i<11; i++) multiFadeNeedsRedraw[i] = true;
  buttonPressed[0] = false;
  buttonPressed[1] = false;
  portConnect();
  // start oscP5, listening for incoming messages at port 8000 
  oscP5 = new OscP5(this,8000);
  // set the local IP address on the iPod to this
  myRemoteLocation = new NetAddress(iPadIP,8080); // local IP on the iPad
  // in choosing a static IP address make sure it is in the same domain as the host
  defineLables();
  updateiPad();
}

void oscEvent(OscMessage theOscMessage) { 
    String addr = theOscMessage.addrPattern();     
     println(addr);   // uncomment for seeing the raw message
    int startOfNumber = addr.indexOf("/push");
    if(startOfNumber != -1) { // we have a push button message
        buttonNumber = getNumber(addr, startOfNumber + 5);
    if(theOscMessage.get(0).floatValue() != 0.0) {  // for button push
         println("press "+ buttonNumber);
         buttonPressed[buttonNumber -1] = true;
        }        
    else {  // for button release
      // println("release "+ buttonNumber);
         buttonPressed[buttonNumber -1] = false;
      }
      multiFadeNeedsRedraw[0] = true; // just redraw fader 0
    }
    // look for fader messages
    if(addr.indexOf("/1/multifader") !=-1){     
      int fader=0;
      String list[] = split(addr,'/');
      if(list[2].length() == 12) fader = int(list[2].substring(10,12));
      if(list[2].length() == 11) fader = int(list[2].substring(10,11));
      fader--; // to compensate for zero based arrays
    //  println(fader);
      int x = int(list[3]);
      multiFade[fader][x]  = theOscMessage.get(0).floatValue();
 //   println(" x1 = "+multiFade[0][x]);  // uncomment to see x value
    multiFadeNeedsRedraw[fader] = true;
    }
}    
 
int getNumber(String s, int look) {
  int number = -1, i = 0;
  char p = '0';
  if(s.length() > look) {
    number = 0;
    for(int k = look; k< s.length(); k++) {
      p = s.charAt(look+i);
      i++;
      if(p >= '0' && p <= '9') number = (number * 10) + (int(p) & 0x0f);
    }
  }
  return(number);
}
  
void draw() {
  // see if screen needs updating
  for(int i=0; i<11; i++){
  if(multiFadeNeedsRedraw[i] == true){
    drawFader(i);  // only redraw the screen if we need to
    multiFadeNeedsRedraw[i] = false;
    }
  }
  // look at push buttons
  if(buttonPressed[1] || saveCallback){ // the save button
     saveCallback = false;
     calculateWaveform(); // work out the waveform tables
     displayWaveforms();
     buttonPressed[1] = false; // show we have done the action
     saveWave(); // save it to disc
  }  
  if(buttonPressed[0] || loadCallback){ // the load button
     loadCallback = false;
     buttonPressed[0] = false; // show we have done the action
     loadWave(); // get it from the disc
  }
}

void calculateWaveform(){
  float sf = scaleFactor();
  float temp;
  // println("scaling factor is " + sf );
  for(int j =0; j<16; j++){
     for(int i = 0; i<256; i++){ // calculate entries in the table
         temp = 0;
         for(int k = 0; k<11; k++){ // for each harmonic
         temp += sf * multiFade[k][j+1] * (sin(i * incSin * (k+1)));
      }
   waveform[j][i] = int(255 * temp );
  }
 }
}

void displayWaveforms(){
  int y=48;
  fill(0,0,0);
  rect(580,0, 850, 790); // blank off previous waveform
  strokeWeight(1);
  noFill();
  stroke(255,255,255);
  for(int table = 0; table <16; table++){
    y = 44 + (48 * table);
  for(int x = 0; x<255; x++){
  point(x+580, y -( waveform[table][x] / 12));
   }
  }
}

float scaleFactor() { // give the overall scale factor for the waveform
float sum, maxSum = 0.0;
  for(int j = 0; j<16; j++){  // go through each table
     sum = 0;
     for(int i = 0; i<11; i++){
       sum += multiFade[i][j+1];  // sum the same
    }
     if(sum > maxSum) maxSum = sum; // get the biggest amplitude
  }
  if(maxSum <= 0.0) maxSum = 1; // prevent an infinity
  return(1.0/maxSum);
}

void drawFader(int fader){ // Update the screen image of the faders
 int xOffset = 137, yOffset = 0;
 color cFill = color(0, 200, 200); // cyan
// draw the load / save buttons
   noStroke();
  if(buttonPressed[0]) fill(180,180,180); else fill(90,90,90);
   rect(65, 48, 32, 32);
  if(buttonPressed[1]) fill(180,180,180); else fill(90,90,90);
   rect(468, 48, 32, 32);
// draw faders
 if(fader != 0){
   xOffset = ((fader+1) % 2) * ( faderWidth + 30);
   yOffset = ((fader+1) / 2) * 130;
   switch(((fader+1) / 2)){
     case 1:
     cFill = color(0, 153, 0); // green
     break;
     case 2:
     cFill = color(200, 200, 0); // yellow
     break;
     case 3:
     cFill = color(153, 0, 153); // purpul
     break;
     case 4:
     cFill = color(240, 64, 0); // orange
     break;
     case 5:
     cFill = color(250, 30 , 30); // red
     break;  
   }
 }
  strokeWeight(2);
  noFill();
  stroke(cFill);
  rect(15+ xOffset, 15 + yOffset, faderWidth + 15, faderHight + 15);  // outline 
  for(int x=1 ; x<17; x++){
    noStroke();
    fill(40,40,40);
    rect( x*16+4+ xOffset, yOffset + 16, 12, faderHight + 13);   // blank rectangle
    fill(cFill); // do the solid square marking the position of the fader
    rect( x*16+4+ xOffset, yOffset + 15+(1-multiFade[fader][x])*faderHight, 12, 12);  
    }   
  calculateWaveform();
  displayWaveforms(); // show the wave at the side of the screen
}

void savePathCall(File selection){
     if (selection == null) {
    // If a file was not selected
    println("No output file was selected...");
   } else {
  // If a file was selected, print path to folder
     savePath= selection.getAbsolutePath();
     waveName = savePath;
     println("save path"+savePath);
     // nibble away at the path name until we just have the file name
     while(waveName.indexOf('/') != -1) { 
        waveName = waveName.substring(1, waveName.length() ); 
      }
     // now make it 8 long by appending spaces
     while(waveName.length() < 8) waveName = waveName + " ";
     output = createWriter(savePath+".asw"); // add file extension
     output.println(waveName); // save the file name as part of the file
     for(int i = 0; i < 11; i++) { // for each harmonic slider
         for(int j = 1; j <17; j++) { // for each slider
            output.println(multiFade[i][j]);
      }
     }
   // Tidy up the file
      output.flush(); // Write the remaining data
      output.close(); // Finish the file
   }
}

void  saveWave(){ // save waveform defination to disc
      selectOutput("select a place to save wavetable","savePathCall");  // Opens file chooser
      // rest of the action handled by call back function
    }
 
 void loadWave(){   // load waveform defination from disc
   selectInput("Choose Waveform file","doLoadWave");  // Opens file chooser
 }
 
 void doLoadWave(File selection){
 int k=1;
   if (selection == null) {
    // If a file was not selected
    println("No file was selected...");
  } else {
    String loadPath = selection.getAbsolutePath();
    println(loadPath);
    lines = loadStrings(loadPath);
    for(int i = 0; i < 11; i++) { // for each harmonic slider
       for(int j = 1; j <17; j++) { // for each slider
          multiFade[i][j] = Float.valueOf(lines[k]);
          k++;
      }
     }
    for(int i =0; i<11; i++) multiFadeNeedsRedraw[i] = true;
    calculateWaveform();
    displayWaveforms(); // show the wave at the side of the screen
    waveName = lines[0];
    while(waveName.length() < 8) waveName = waveName + " "; // for files with a short wave name
    // println("Waveform name is " + lines[0]);     
    updateiPad(); // update the iPad
  }
 }
 
 void updateiPad(){// update the iPad
     String message, finalMessage;
     for(int i=0; i<11; i++) { // for each fader
        message = "/1/multifader" + str(i+1);
        for(int j = 1; j<17; j++) { // for each silder in a fader
           finalMessage = message + "/" + str(j);
           OscMessage myMessage = new OscMessage(finalMessage);
           myMessage.add(multiFade[i][j]); // add an float to the osc message
           oscP5.send(myMessage, myRemoteLocation);
           bufDelay(4); // make sure we don't do thing too fast
     } // end of each slider in a fader
     } // end of for each fader
 }
  
 void portConnect(){    // Open the port that the SpoonDuino is connected to and use the same speed
       // **********************************
      // if the device you are looking for is 
      // not avaliable the program will 
      // connect to the first one in the list
      // ************************************
      int portNumber = 99;
      String [] ports;
      // println(Serial.list()); // uncomment for full list of serial devices
      ports = Serial.list();
      for(int j = 0; j< ports.length; j++) { 
         if(adaptor.equals(Serial.list()[j])) portNumber = j;         
      } // go through all ports
      if(portNumber == 99) portNumber = 0; // if we haven't found our port connect to the first port
      String portName = Serial.list()[portNumber]; 
      println("Connected to "+portName);
      // port = new Serial(this, portName, 57600);
      port = new Serial(this, portName, 38400);
      port.bufferUntil(10);  // call serialEvent every line feed
   }
  
  void serialEvent(Serial port) {  // this gets called everytime a line feed is recieved
  String recieved = port.readString() ;  
  // println(recieved + " from serial port");
  String startTransfer [] = match(recieved,"send");
  if(startTransfer != null) sendWave();
    else println(recieved);
}

void sendWave(){ // send wave table to the SpoonDuino
  float sf = scaleFactor();
  float temp, maxTemp = -2, minTemp = 2;
  int entry, count=0;
  println("now sending " + waveName);
  for(int i = 0; i<8; i++) port.write(waveName.charAt(i));
  println("scaling factor is " + sf );
  for(int j =0; j<16; j++){
     for(int i = 0; i<256; i++){ // calculate entries in the table
        temp = 0;
        for(int k = 0; k<11; k++){ // for each harmonic
          temp += sf * multiFade[k][j+1] * (sin(i * incSin * (k+1)));
      }
    if(temp > maxTemp) maxTemp = temp;
    if(temp < minTemp) minTemp = temp;
    entry = 2048 + int(2047 * temp ); 
    count++;
    port.write((entry >> 8) & 0xff); // send MSB
    port.write(entry & 0xff); // send LSB
   }
  }
  println(" ");
  println("Entry count is "+ count);
  println("max value "+ maxTemp + " minimum value " + minTemp);
}

void defineLables(){
    synth = new ControlP5(this);
  loadLab = synth.addTextlabel("label1")
                 .setText("Load")
                 .setPosition(66,36)
                 .setColorValue(0xfff0f0f0);
  saveLab = synth.addTextlabel("label2")
                 .setText("Save")
                 .setPosition(470,36)
                 .setColorValue(0xfff0f0f0);
   fund =  synth.addTextlabel("label3")
                .setText("Fundamental")
                .setPosition(256,4)
                .setColorValue(0xff00c8c8);
   h2 =    synth.addTextlabel("label4")
                .setText("2nd")
                .setPosition(135,130)
                .setColorValue(0xff009900);
   h3 =    synth.addTextlabel("label5")
                .setText("3rd")
                .setPosition(420,130)
                .setColorValue(0xff009900);
   h4 =    synth.addTextlabel("label6")
                .setText("4th")
                .setPosition(135,260)
                .setColorValue(0xffc8c800);
   h5 =    synth.addTextlabel("label7")
                .setText("5th")
                .setPosition(420,260)
                .setColorValue(0xffc8c800);
   h6 =    synth.addTextlabel("label8")
                .setText("6th")
                .setPosition(135,390)
                .setColorValue(0xff990099);
   h7 =    synth.addTextlabel("label9")
                .setText("7th")
                .setPosition(420,390)
                .setColorValue(0xff990099);
   h8 =    synth.addTextlabel("label10")
                .setText("8th")
                .setPosition(135,520)
                .setColorValue(0xfff04000);
   h9 =    synth.addTextlabel("label11")
                .setText("9th")
                .setPosition(420,520)
                .setColorValue(0xfff04000);
   h10 =   synth.addTextlabel("label12")
                .setText("10th")
                .setPosition(135,650)
                .setColorValue(0xfffa1e1e);
   h11 =   synth.addTextlabel("label13")
                .setText("11th")
                .setPosition(420,650)
                .setColorValue(0xfffa1e1e);
}

void mousePressed() {
 int x,y;
  x = mouseX; 
  y = mouseY; 
  // println(x+" "+y);
  if(x>67 && x<97 && y>50 && y<82) {
    mouseHit= true;
    noStroke();
    fill(180,180,180);
    rect(65, 48, 32, 32);
    loadCallback = true;
  }
  if(x>470 && x< 499 && y>50 && y<82){
    mouseHit = true;
    noStroke();
    fill(180,180,180);
    rect(468, 48, 32, 32);
   saveCallback = true;
  }
}
void mouseReleased() {
  if(mouseHit){
    mouseHit = false;
    noStroke();
    fill(90,90,90);
    rect(65, 48, 32, 32);
    rect(468, 48, 32, 32);
  }
}
 void bufDelay(long pause){
   pause = pause + millis();
   while(pause > millis()) { } // do nothing
 }
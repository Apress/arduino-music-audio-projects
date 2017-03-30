/* SpoonDuino - a musical instrument
*  Waveform table synthisister with spoon playing device
* By Mike Cook Nov 2015
*/
#include <SPI.h>
#include <LCD_I2Cm.h>
#include <EEPROM.h>
#include <I2C.h>
  
#define Xpad1 4
#define Xpad2 3
#define Ypad1 2
#define Ypad2 7
#define spoon1 0
#define blueLED 5
#define greenLED 6
#define redLED 17
#define keys 2
#define CS_BAR 9
#define LATCH 8

#define SR_CS 10  // static RAM chip select bar
#define SR_SI 12  // serial data input
int memoryAddresBase = 0x50;
 
// initialize the library with the I2C address
LCD_I2Cm lcd(0x20); // send I2C address for PCF8574A with external address lines = 0
boolean button[4];     // array for menu buttons
boolean lastButton[4];
int menu =0, menuMax = 3; 
String menuTitle [] = { "Play    ", "Load ", "Save ", "Get Wave"};
// autoIncDelay - delay before auto increment kicks in
// autoIncPeriod - speed of auto increment
long unsigned int autoIncDelay = 800, autoIncPeriod = 150, autoIncTrigger=0, autoInc=0;
int value[] = { 3, 0, 0, 0};   // initial values
int valueMax [] = { 3, 31, 31, 0}; // maximum value
int valueMin [] = {0, 0, 0, 0};  // mimimum value
// increment for notes C1      D1      E1      F1      G1     A1       B1      C2      D2      E2      F2
int noteLookup[] = { 0x1125, 0x133e, 0x159a, 0x16e3, 0x19b0, 0x1cd6, 0x205e, 0x224a, 0x267d, 0x2b34, 0x2dc6 }; // increments
String playMenu [] = { "Shot    ", "Loop    ", "Static  ", "Static Q"}; // choice of playing options
char waveName [] = { 'b', 'l', 'a', 'n', 'k', ' ', ' ', ' ', ' '}; // array for wave name
int rawButton;
int xVal1, xVal2, yVal1, yVal2, thresh1, thresh2, key;
boolean retrigger = false;
// ISR variables
 volatile long int index = 0;
 volatile long int increment = 0x700;
 volatile boolean hush = false;
 volatile int tableOffset = 0;
 boolean tempHush = false;

void setup()
{
  // initilise control pins for A/D
  pinMode(LATCH, OUTPUT);
  digitalWrite(LATCH, HIGH);
  pinMode(CS_BAR, OUTPUT);
  digitalWrite(CS_BAR, HIGH);
    // initilise control pins for SRAM
  pinMode( SR_CS, OUTPUT);
  digitalWrite( SR_CS, HIGH);
  pinMode( SR_SI, INPUT);
   // initilise LED outputs
  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  digitalWrite(redLED, LOW); // turn on red light
  digitalWrite(greenLED, HIGH);
  digitalWrite(blueLED, HIGH);

  Serial.begin(38400);  // start serial for output
  I2c.begin();
  I2c.setSpeed(1);   // my hardware would only work at 800KHz
  Serial.println("SpoonDuino running");  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  // set status register to byte mode
   digitalWrite( SR_CS,LOW);
   SPI.transfer(0x01);  // write to status register
   SPI.transfer(0x41);  // page mode with hold disabled
   digitalWrite( SR_CS,HIGH); 
   SPI.setClockDivider(SPI_CLOCK_DIV2); // maximum clock speed 
   lcd.begin(8, 2);   
  // Print introductory message to the LCD.
   lcd.print("SpoonDui");
   lcd.setCursor(0,1);
   lcd.print("no");  
   delay(1000);
   lcd.setCursor(0,0);
   lcd.print("By Mike ");
   lcd.setCursor(0,1);
   lcd.print("Cook");
   delay(1500);  
   lcd.clear();
   
   menuSteup(menu);  // start off the menu system
   autoIncTrigger = millis();
   hush = true;
   // automatically load in the waveform in slot zero
   romReadWave(0);
   readWaveName(0);
   // set the ISR going
   setSampleTimer();
   TIMSK2 = _BV(OCIE2B); // Output Compare Match B Interrupt Enable
}

boolean dir = true;
long tableTime=0;
long tableShiftRate = 100;

void loop(){
    doMenu();
    saveButtons();
    if(millis() > tableTime) {
      if(value[0] == 0 || value[0] == 1) { // if we are in playing modes shot or loop
      if(!hush) {  // if we are curently not playing
    if(dir) tableOffset += 512; else tableOffset -= 512;
    if(tableOffset >= 7680) { 
      dir = false;
      if(value[0] == 0) hush = true; // stop going on shot mode
     }
      if(tableOffset <= 0) dir = true;
  }
    spoonRead();
    if(hush) tableTime = millis() + 100; else tableTime = millis() + tableShiftRate;
}
    else { // if we are in another mode , just static or static Q mode for the moment
    spoonRead();
    tableTime = millis() + 50; // update in another 50mS
    }    
  }
}


ISR(TIMER2_COMPB_vect){  // Interrupt service routine to read the sample and send it to the A/D
    if(!hush){ // if playing a note then output the next sample
      index += increment;
      outA_D(ramRead(tableOffset + ((index>>8) & 0x1fe)) );
    }
}

void setSampleTimer(){  // sets timer 2 going at the output sample rate
  TCCR2A = _BV(WGM21) | _BV(WGM20); // Disable output on Pin 11 and Pin 3
  TCCR2B = _BV(WGM22) | _BV(CS22);
  OCR2A = 124; // defines the frequency 120 = 16.13 KHz or 62uS, 124 = 15.63 KHz or 64uS, 248 = 8 KHz or 125uS
  TCCR2B = TCCR2B & 0b00111000 | 0x2; // select a prescale value of 8:1 of the system clock
}

static int workingValue;
void doMenu(){
   if(readButtons()) {
     if(!button[0] && lastButton[0]){ // on menu change button
       menu ++;
       if(menu > menuMax) menu = 0;
       if(menu == 2) value[2] = value[1];
       workingValue = value[menu];
       menuSteup(menu);
    }
      
    if(!button[1] && lastButton[1]){ // Accept this menu choice
      value[menu] = workingValue;  // save new value
      // do the actions depending on the choice and menu
      tempHush = hush;
      hush = true;
      if(menu == 1) {
        lcd.setCursor(0, 0);
        lcd.print("Loading ");
        romReadWave(workingValue);
        readWaveName(workingValue);
         }
      if(menu == 2) {
        lcd.setCursor(0, 0);
        lcd.print("Saving ");
        romWriteWave(workingValue);
        romWaveName(workingValue);
         }
       if(menu == 3) {  // transfer wave table from Processing
          getWaveTable();
          }
       hush = tempHush;
        menuSteup(menu); // restore menu
      }  
      
   if(!button[2] && lastButton[2]) {
      if(workingValue > valueMin[menu]) workingValue--; else workingValue = valueMax[menu];
      updateValue(menu, workingValue);
      value[menu] = workingValue;
   }
     if(!button[3] && lastButton[3]) {
      if(workingValue < valueMax[menu]) workingValue++; else workingValue = valueMin[menu];
      updateValue(menu, workingValue);
      value[menu] = workingValue;
   } 
 }
}

void updateValue(int m, int v){
    if(m == 0) {lcd.setCursor(0, 1); lcd.print(playMenu[v]);}
    
  if(m == 1) { // load menu
   lcd.setCursor(5, 0); 
   lcd.print(v); 
   lcd.spc(2);
   displayLoadWaveName(v);
  }
  
  if(m ==2) { // save menu
    lcd.setCursor(5, 0); 
    lcd.print(v); 
    lcd.spc(2);
    displayWaveName(); 
  }

}

void menuSteup(int n){
    lcd.setCursor(0, 0);
    lcd.print(menuTitle[n]);  
    if(n == 0) { lcd.spc(2); lcd.setCursor(0, 1); lcd.print(playMenu[value[0]]); lcd.spc(1);}
    if(n == 1){ // load menu
       lcd.print(value[1]);
       lcd.spc(2); // blank off any other value remnents
       displayLoadWaveName(value[1]);
     }

     if(n ==2){ // save menu
       lcd.print(value[2]);
       lcd.spc(2); // blank off any other value remnents
       displayWaveName(); 
     }
     if(n ==3){ // Get wave menu
     lcd.setCursor(0, 1);
     lcd.spc(8);  // blank off any name
     }
}

void readButton(int b){

  boolean pressed = true;
  switch (b) {
    case 0:
    if((rawButton > 0xc9) &&  (rawButton < 0xE0) ) pressed = false;
    break;
    case 1:
    if((rawButton > 0x158) &&  (rawButton < 0x168) ) pressed = false;
    break;
    case 2:
    if(rawButton >= 512) pressed = false;
    break;
    case 3:
    if((rawButton > 0x60) &&  (rawButton < 0x80) ) pressed = false;
    break;
  }
 button[b] = pressed;

}


boolean readButtons(){
  boolean change = false;
  rawButton = analogRead(keys) + 8;
  for(int i=0; i<4; i++){
  readButton(i);
   if(button[i] != lastButton[i]){ 
      if( ( i == 2 || i == 3) && button[i] == LOW) { // start off auto increment timer
      autoIncTrigger = millis();
      autoInc = millis();
      }
      delay(30); // debounce delay
      change= true; 
     } // debounce delay if a change
  }
  // if(change)Serial.println("button change");
  if(  (button[3] == LOW || button[2] == LOW) && ( (millis() - autoIncTrigger) > autoIncDelay) ) { // need to auto increment
    if(millis() - autoInc > autoIncPeriod) {  // do the auto increment
      autoInc = millis();
      if(button[2] == LOW) {
      if(workingValue > valueMin[menu]) workingValue--; // don't wrap round under auto increment
      updateValue(menu, workingValue);
        }
     if(button[3] == LOW) {
      if(workingValue < valueMax[menu]) workingValue++; // don't wrap round under auto increment
      updateValue(menu, workingValue);
       }       
    } // end of do the auto increment
  }
  return change;
}

void saveButtons(){
  for(int i=0; i<4; i++){
    lastButton[i] = button[i];
  }
}

void ramWrite(int add, int val){ // write val to address add of the SRAM as two bytes
  // digitalWrite( SR_CS,LOW);
   PORTB = PINB & 0xfb;
  SPI.transfer(0x02);  // write data to memory instruction
  SPI.transfer((add>>8) & 0x7f );
  SPI.transfer(add & 0xff);
  SPI.transfer(val>>8);  // write MS nibble first
  SPI.transfer(val & 0xff);
 // digitalWrite( SR_CS,HIGH); 
  PORTB = PINB | 0x04; 
}

void singleRamWrite(int add, uint8_t val){ // write val to address add as single byte
  // digitalWrite( SR_CS,LOW);
   PORTB = PINB & 0xfb;
  SPI.transfer(0x02);  // write data to memory instruction
  SPI.transfer((add>>8) & 0x7f );
  SPI.transfer(add & 0xff);
  SPI.transfer(val);  // write 
 // digitalWrite( SR_CS,HIGH); 
  PORTB = PINB | 0x04; 
}

int ramRead(int add){ // read val from address as two bytes
int val;
  // digitalWrite( SR_CS,LOW); // pin 10
  PORTB = PINB & 0xfb;
  SPI.transfer(0x03);  // read data from memory instruction
  SPI.transfer((add>>8) & 0x7f);
  SPI.transfer(add & 0xff);
  val = SPI.transfer(0) << 8; // read most significant nibble
  val |= SPI.transfer(0);
//  digitalWrite( SR_CS,HIGH); 
  PORTB = PINB | 0x04;
  return val;
}

uint8_t singleRamRead(int add){ // read val from address as two bytes
uint8_t val;
  // digitalWrite( SR_CS,LOW); // pin 10
  PORTB = PINB & 0xfb;
  SPI.transfer(0x03);  // read data from memory instruction
  SPI.transfer((add>>8) & 0x7f);
  SPI.transfer(add & 0xff);
  val = SPI.transfer(0);
//  digitalWrite( SR_CS,HIGH); 
  PORTB = PINB | 0x04;
  return val;
}

void outA_D(int value){
  int first;
      first = ( (value >> 8) &0x0f )        |  0x40 |    0x20 |       0x10;
    //                        side A  |bufferd| gain 1 | output enabled
   // take the SS pin low to select the chip:
 // digitalWrite(CS_BAR,LOW); // pin 9
   PORTB = PINB & 0xfd;
  SPI.transfer(first);      //control and MS nibble data
  SPI.transfer(value & 0xff);  // LS byte of data
  //  digitalWrite(CS_BAR,HIGH);
  PORTB = PINB | 0x02; 
 //   digitalWrite(LATCH, LOW);   // latch the output pin 8
 //   digitalWrite(LATCH, HIGH); 
  PORTB = PINB & 0xfe;
  PORTB = PINB | 0x01; 
}

void displayWaveName(){
   // send wave name to LCD
        lcd.setCursor(0, 1);
        for(int i =0; i<8; i++){
       lcd.print(waveName[i]);
        }
  }
  
void getWaveTable(){  // transfer from processing into the memory
int val,address=0;
       lcd.setCursor(0, 0);
        lcd.print("Getting ");
        Serial.println("send");
        for(int numberOfBytes = 0; numberOfBytes < 8200; numberOfBytes++){ // read all the bytes
        while(Serial.available() == 0){ } // hold until data is in
         if(numberOfBytes < 8) { // get the waveform name
         waveName[numberOfBytes] = Serial.read();
           }
          else { // get the waveform tables
          if(address & 1){ // on odd addresses write to memory
           val |= Serial.read(); 
           ramWrite(address-1, val); // save it in memory
           }
          else { // on even addreses just get the most significant byte
          val = Serial.read() << 8;
          }
          address++;
          } 
        } // end of reading all the bytes
      menu = 2; // change to save menu 
 }

void romWriteWave(int number){  // write a waveform from RAM to ROM
   int address, pg, bufferNumber=0;
   long int time;
   uint8_t buffer[255];
   for(int page= 0; page<32; page++){  // write waveform over 32 pages
   pg = (number * 32) + page; // get absoloute point in eeprom
   address = memoryAddresBase | ( pg >> 8);
   // now fill up the buffer
   for(int i=0; i<256; i++) buffer[i] = singleRamRead(i+(bufferNumber << 8));
   bufferNumber++;
   // now write it to EEPROM
   while(I2c.write2(address, pg & 0xff, 0, buffer, 256) != 0 ) { } // repeat until command is taken
   }
  }
  
void romReadWave(int number){  // read a waveform from ROM to RAM
  int c, address, pg, ramAddress = 0;
     for(int page= 0; page<32; page++){  // write waveform over 32 pages
     pg = (number * 32) + page; // get absoloute point in eeprom
     address = memoryAddresBase | ( pg >> 8); // create the I2C address of the chip to use
   for(int i=0; i<16; i++){  // read 16 lines for the page
      while(I2c.write(address,pg & 0xff, i<<4) != 0) { }  // set up start of read
      I2c.read(address, 16);      // request 16 bytes from memory device ( note maximum buffer size is 32 bytes )
      for(int j=0; j<16; j++){   // now get the bytes one at a time
      singleRamWrite(ramAddress, I2c.receive());     // receive a byte
      ramAddress++; // move on to next address
       }
     }
   }
}   
   
void romWaveName(int slot){
  slot = slot << 3; // make it into an eprom address
  for(int i=0; i < 8; i++){ // store the name in internal EEPROM
    EEPROM.write(slot + i, waveName[i]);
  }
}

void displayLoadWaveName(int slot){
    slot = slot << 3; // make it into an eprom address
    lcd.setCursor(0, 1);
      for(int i=0; i < 8; i++){ // get the name in internal EEPROM
      lcd.print((char)EEPROM.read(slot + i));
  }
}

void readWaveName(int slot){
    slot = slot << 3; // make it into an eprom address
  for(int i=0; i < 8; i++){ // get the name in internal EEPROM
   waveName[i] = EEPROM.read(slot + i);
  }
}

void spoonRead(){
  key = analogRead(keys);
  if(touching()) { // read the spoon co-ordnates if we are touching the pad
      digitalWrite(redLED, HIGH); // LED off
      analogWrite(blueLED, xVal2 >>2);
         // take a reading
         setPads(true, false);
         xVal1 = analogRead(spoon1);
         setPads(false, false);
         yVal1 = analogRead(spoon1);

      switch(value[0]){
       case 0: // playing mode Shot or loop
       if(!retrigger) break; // exit here if we have not had a retrigger here yet
       tableOffset = 0; // start over
       dir = true;  // going up
       retrigger = false;
       // - no I haven't missed out the break I want it to be like this
       case 1:
         hush = false;
         tableShiftRate = (yVal1 - 130)/8;
         increment = (xVal1 - 80) << 4;
      break;
      case 2: // static mode
      case 3: // static quantisation mode
      hush = false;
      tableOffset = ((yVal1 - 130)/50) << 9; // make it a whole number of tables
      if(tableOffset < 0) tableOffset =0;
      if(tableOffset > 7680) tableOffset = 7680;
      if(value[0] == 2)increment = (xVal1 - 80) << 4; else increment = quantIncrement();
      break;
      }
    }
   else {
     digitalWrite(redLED, LOW); // LED on
     analogWrite(blueLED, 255); // LED off
     switch(value[0]) {
       case 0:
       retrigger = true;
       break;
       case 1: // playing mode loop
       case 2: // static mode
       case 3: // static quantised mode
       hush = true;
       break;
     }
   } 
}

long int quantIncrement(){
  int quant;
  quant = (xVal1 - 80)/88;
  return noteLookup[quant];
}

boolean touching(){
  boolean touch1 = false, touch2 = false;
  ADMUX = 0x4F; // select channel 15, this puts the input mux capacitor to ground
  // Set all the pads to outputs
   pinMode(Ypad1, OUTPUT);
   pinMode(Ypad2, OUTPUT);
   pinMode(Xpad1, OUTPUT);
   pinMode(Xpad2, OUTPUT);
   // put all the pads high
  digitalWrite(Xpad1, HIGH);
  digitalWrite(Xpad2, HIGH);
  digitalWrite(Ypad1, HIGH);
  digitalWrite(Ypad2, HIGH);
  if( analogRead(spoon1) > 980) touch1 = true;
     // put all the pads low
     digitalWrite(Xpad1, LOW);
     digitalWrite(Xpad2, LOW);
     digitalWrite(Ypad1, LOW);
     digitalWrite(Ypad2, LOW);
  if(analogRead(spoon1) < 25) touch2 = true;
  return (touch1 && touch2);
}

void setPads(boolean way, boolean pol){    // initilise xy Pads
    ADMUX = 0x4F; // select channel 15, this puts the input mux capacitor to ground
    delay(1);
  if(way){
    pinMode(Ypad1, INPUT);
    pinMode(Ypad2, INPUT);
    pinMode(Xpad1, OUTPUT);
    pinMode(Xpad2, OUTPUT);
  if(pol){
    digitalWrite(Xpad1, LOW);
    digitalWrite(Xpad2, HIGH);
   } else {
    digitalWrite(Xpad1, HIGH);
    digitalWrite(Xpad2, LOW);
   }
  }
    else {
      pinMode(Xpad1, INPUT);
      pinMode(Xpad2, INPUT);
      pinMode(Ypad1, OUTPUT);
      pinMode(Ypad2, OUTPUT);
   if(pol){
      digitalWrite(Ypad1, LOW);
      digitalWrite(Ypad2, HIGH);
    } else {
      digitalWrite(Ypad1, HIGH);
      digitalWrite(Ypad2, LOW);
    }    
 }
}


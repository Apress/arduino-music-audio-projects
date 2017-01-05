// LCD I2C library for Arduino 1.0
// Standard LCD - added I2C + Back light control Mike Cook Jan 2012
// For the PCF8574 single byte port expanders
// m version for use with the I2C Master library

#include "LCD_I2Cm.h"

// #include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"
// #include <Wire.h>
#include <I2C.h>

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    4-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
// 5. Backlight output at zero
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LCD_I2C constructor is called).


LCD_I2Cm::LCD_I2Cm(uint8_t address)
{
  init( address);
}

void LCD_I2Cm::init(uint8_t address)
{

   i2cAddress = address;
   i2cData = 0x00;
   // Wire.begin();
   I2c.begin(); // start up the master I2C library
   _displayfunction = LCD_1LINE | LCD_5x8DOTS;
}

void LCD_I2Cm::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;
  _currline = 0;


  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  delayMicroseconds(50000); 
  // Now we pull both RS and R/W low to begin commands
     i2cWrite(_RS | _EN | _RW, false);
   
  //put the LCD into 4 bit mode
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms
    
    // third go!
    write4bits(0x03); 
    delayMicroseconds(150);

    // finally, set to 4-bit interface
     write4bits(0x02); 

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

}

/********** high level commands, for the user! */
void LCD_I2Cm::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  holdReady();  // delay until LCD is ready again
 //  delayMicroseconds(2000);  // this command takes a long time!
}

void LCD_I2Cm::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
   holdReady();  // delay until LCD is ready again
 // delayMicroseconds(2000);  // this command takes a long time!
}

void LCD_I2Cm::setCursor(uint8_t col, uint8_t row)
{
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row >= _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
  }
  
  command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCD_I2Cm::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_I2Cm::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LCD_I2Cm::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_I2Cm::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LCD_I2Cm::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_I2Cm::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCD_I2Cm::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCD_I2Cm::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCD_I2Cm::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LCD_I2Cm::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LCD_I2Cm::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LCD_I2Cm::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Set the back light control bit (bit 7 of expander) high or low
void LCD_I2Cm::backLight(uint8_t value){
  i2cWrite(_BL, value); // sets the back light bit 
}

// writes a given number of blank spaces on the display
void LCD_I2Cm::spc(uint8_t number){
  for(int i=0; i< number; i++) write(0x20);
 }

// writes blank spaces on the display until the address of the number given
void LCD_I2Cm::blankTo(uint8_t col, uint8_t row){
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row >= _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
  }
  int number = col + row_offsets[row]; // where we want to get to in the display memory
  int current = readAddress(); // get display address we are at
  if((number - current) > 0) spc(number - current);  // write the appropriate number of blanks
 }


// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCD_I2Cm::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

inline void LCD_I2Cm::command(uint8_t value) {
  send(value, false);
}

inline size_t LCD_I2Cm::write(uint8_t value) {
  send(value, true);
  return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void LCD_I2Cm::send(uint8_t value, uint8_t mode) {
    i2cWrite(_RS, mode);
  // set RW low to Write
    i2cWrite(_RW, false);
  
    write4bits(value>>4);
    write4bits(value);  
}

void LCD_I2Cm::pulseEnable(void) {
  i2cWrite(_EN, true);
  // enable pulse must be >450ns it is because of 100KHz I2C bus
  i2cWrite(_EN, false);
  delayMicroseconds(100);   // commands need > 37us to settle
}

void LCD_I2Cm::write4bits(uint8_t value) {

   i2cData = (i2cData & 0xf0) | (value & 0x0f) ;
   i2cWrite(0, true); // dummy parameters to get the i2cData to be written
  pulseEnable();
}

void LCD_I2Cm::i2cWrite(uint8_t bits, bool state){  // write to device
   // prepare data byte
   if(state) i2cData |= bits; else i2cData &= ~bits; 
   // write it
/*
   Wire.beginTransmission(i2cAddress); // transmit to device
   Wire.write(i2cData);              // sends one byte 
   Wire.endTransmission();    // stop transmitting 
*/
  I2c.write(i2cAddress, i2cData);
}

// reads the current address of the cursor
uint8_t LCD_I2Cm::readAddress() {   
  i2cData = (i2cData & _BL) | 0xf | _RW;  // set the data pin bits to inputs, set to read (HIGH) _RS is low
  i2cWrite(0, true); // dummy parameters to get the i2cData to be written
   uint8_t ad;
         i2cWrite(_EN, true);
         ad = (i2cRead() & 0x7) << 4;  // mask out busy flag and all the others
         i2cWrite(_EN, false);
         i2cWrite(_EN, true); 
         ad |= (i2cRead() & 0xf);
         i2cWrite(_EN, false);
  
        i2cWrite(_RW, false);  // set to write - LCD data bits to inputs
     // no need to set the data pin bits to outputs
  return ad; // return the address
}

// reads LCD busy flag and holds until it is ready
void LCD_I2Cm::holdReady() {   
  i2cData = (i2cData & _BL) | 0xf | _RW;  // set the data pin bits to inputs, set to read (HIGH) _RS is low
  i2cWrite(0, true); // dummy parameters to get the i2cData to be written
   uint8_t bf;
   do {
         i2cWrite(_EN, true);
         bf = i2cRead() & 0x8;  // busy flag
         i2cWrite(_EN, false);
         pulseEnable(); // so do one more pulse of the enable pin to finish the op
  } while (bf);
  
        i2cWrite(_RW, false);  // set to write - LCD data bits to inputs
     // no need to set the data pin bits to outputs
}

uint8_t LCD_I2Cm::i2cRead(){  // read device
  // Wire.requestFrom(int(i2cAddress), 1);    // request 1 byte from slave device
   I2c.read(int(i2cAddress), 1);    // request 1 byte from slave device
   return(I2c.receive()); // return it
}

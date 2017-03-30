/*
  LiquidCrystal Library - BlankTo
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
 This sketch prints "Hello World!" to the LCD
 and counts in words. 
 It shows the use of the blanks command to tidy up the end of the previous word that is not
 over written by the new one. This saves on string space.
 The blankTo(col, row) function puts blanks from the current cursor position until the cursor position given
 
 The circuit:
 * LCD RS pin to output 4 pin 9 of PCF8574
 * LCD Enable pin to output 6 pin 11 of PCF8574
 * LCD D4 pin to output 0 pin 4 of PCF8574
 * LCD D5 pin to output 1 pin 5 of PCF8574
 * LCD D6 pin to output 2 pin 6 of PCF8574
 * LCD D7 pin to output 3 pin 7 of PCF8574
 * LCD R/W pin to output 5 pin 10 of PCF8574
 * LCD backlight through a transistor output 7 pin 12 of PCF8574 (optional)
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 I2C interface added Mike Cook Jan 2012
 
 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
 #include <LCD_I2C.h>

// initialize the library with the I2C address
LCD_I2C lcd(0x38); // send I2C address for PCF8574A with external address lines = 0
long int time;  // when to print
int number = 0; // what to pront
String words [] = {"zero","one","two","three","four","five","six","seven","eight","nine"};

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world - yes you");
  time = 1000 + millis(); // when to print next
  delay(1000);
  lcd.setCursor(0, 0); // back to the start of the display
  lcd.print("Counting");
  lcd.blankTo(22, 0); // remove the last bit of the last message
}

void loop() {
  if(millis() > time){
  time = 1000 + millis(); // when to print next
  lcd.setCursor(0, 1);
  lcd.print(words[number]);
  // comment out this next line to see what happens without it
  lcd.spc(2); // follow by 2 blanks to erase what was on the display already
  number++; // move to the next number
  if(number >= 10) number = 0; // wrap round the number
  }
}


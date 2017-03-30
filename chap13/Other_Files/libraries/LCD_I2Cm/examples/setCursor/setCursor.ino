/*
  LiquidCrystal Library - setCursor
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
 This sketch prints to all the positions of the LCD using the
 setCursor(0 method:
 
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

// these constants won't change.  But you can change the size of
// your LCD using them:
const int numRows = 2;
const int numCols = 16;

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(numCols,numRows);
}

void loop() {
  // loop from ASCII 'a' to ASCII 'z':
  for (int thisLetter = 'a'; thisLetter <= 'z'; thisLetter++) {
    // loop over the columns:
    for (int thisCol = 0; thisCol < numRows; thisCol++) {
      // loop over the rows:
      for (int thisRow = 0; thisRow < numCols; thisRow++) {
        // set the cursor position:
        lcd.setCursor(thisRow,thisCol);
        // print the letter:
        lcd.write(thisLetter);
        delay(200);
      }
    }
  }
}



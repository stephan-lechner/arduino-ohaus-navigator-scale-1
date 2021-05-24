/*
 Serial communication (RS-232) with an Ohaus Navigator scale

 Demonstrates how an Arduino Mega 2560 can receive the weight from a scale with an RS-232 interface. The weight is used
 for different purposes:
 - Received data is displayed on a 16x2 LCD (top row).
 - A capacity bar is displayed on the bottom row (shows how much of the max. capacity of the scale is being used).
 - Three LEDs indicate whether the weight is below a lower limit (see const loLimit), between the lower and upper limit
 or above the upper Limit (const hiLimit). This can be useful for check weighing.

 This sketch is meant to be used with an Ohaus Navigator scale. Please change the following settings in the menu of the
 scale:
 Print>APrint>Cont (continuous auto-print)
 RS232>baud>9600
 RS232>parity>8 none

 Set the unit to gram (g).

 Please do not contact the author for help when using a different scale. Such messages will be ignored.

 WARNING! DO NOT CONNECT YOU ARDUINO DIRECTLY TO THE RS232 INTERFACE OF A SCALE! You have to use a MAX232 or similar
 signal converter.

 Created May 24, 2021 by Stephan Lechner

 This sketch uses the LCD example code from: http://www.arduino.cc/en/Tutorial/LiquidCrystalSerialDisplay
*/

#include <LiquidCrystal.h>

// LCD
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// serial communication with the scale
const int maxChars = 42;  // Ohaus Navigator should not send more than 41 characters for each weight value, +1 for string terminator
char receivedStr[maxChars];
int numCharsReceived = 0;  // required in the loop to count received characters

// check weighing with 3 LEDs
const float loLimit = 4990;  // lower limit in g
const float hiLimit = 5010;  // upper limit in g
const int pinHiLed = 50;
const int pinOkLed = 48;
const int pinLoLed = 46;

// capacity bar
const int maxWeight = 6200;  // depends on the model of your scale

// displays a String in the first row of the LCD
// does not use lcd.clear to prevent flickering
void displayString(char str[]) {
  lcd.setCursor(0, 0);
  int num = strlen(str);
  for (int i = 0; i < 16; i++) {  // 16 characters per row
    if (i < num)
      lcd.write(str[i]);
    else
      lcd.write(" ");  // fill remaining characters
  }
}

// check weighing logic, three LEDs are used to signal three possible states LOW / OK / HIGH
void checkWeigh(float weight) {
  //check LOW
  if (weight < loLimit) {
    digitalWrite(pinHiLed, LOW);
    digitalWrite(pinOkLed, LOW);
    digitalWrite(pinLoLed, HIGH);
    return;
  }
  //check HIGH
  if (weight > hiLimit) {
    digitalWrite(pinHiLed, HIGH);
    digitalWrite(pinOkLed, LOW);
    digitalWrite(pinLoLed, LOW);
    return;
  }
  // must be OK
  digitalWrite(pinHiLed, LOW);
  digitalWrite(pinOkLed, HIGH);
  digitalWrite(pinLoLed, LOW);
}

// capacity bar - note that it does not consider a possible tare weight
void updateBar(float weight) {
  lcd.setCursor(0, 1);
  int num = round(weight / maxWeight * 16);  // 16 segments
  for (int i = 0; i < 16; i++) {             // 16 characters per row
    if (i < num)
      lcd.write('=');
    else
      lcd.write(" ");  // fill remaining characters
  }
}

void setup() {
  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);

  // initialize serial communication
  Serial1.begin(9600);
  // check weighing LEDs
  pinMode(pinHiLed, OUTPUT);
  pinMode(pinOkLed, OUTPUT);
  pinMode(pinLoLed, OUTPUT);
}

void loop() {
  // read data from serial port 1 on Arduino Mega
  while (Serial1.available() > 0) {
    char c = Serial1.read();                               // read one character
    if (c == '\n' || numCharsReceived == maxChars) {       // last character ('\n' terminator) found or max length reached
      if (receivedStr[strlen(receivedStr) - 1] == '\r') {  //if '\r' found
        receivedStr[strlen(receivedStr) - 1] = '\0';       // remove by overwriting with null terminator
      }
      displayString(receivedStr);
      float weight = atof(receivedStr);  // convert weight string to float
      updateBar(weight);                 // update capacity bar
      checkWeigh(weight);                // update check weighing LEDs
      receivedStr[0] = '\0';             // reset string by terminating at position 0
      numCharsReceived = 0;
    } else {
      // end not reached yet, add received character to string
      receivedStr[numCharsReceived] = c;
      receivedStr[numCharsReceived + 1] = '\0';
      numCharsReceived++;
    }
  }
}

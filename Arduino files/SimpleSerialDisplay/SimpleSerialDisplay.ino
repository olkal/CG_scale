/*
 * Simple LCD Serial display
 * Olav Kallhovd 2016
 * 
  LCD Serial Input, Cursor command: 254, then 128-207
  
  position        1 	2 	3 	4 	5 	6 	7 	8 	9 	10 	11 	12 	13 	14 	15 	16
  line      0 	128 	129 	130 	131 	132 	133 	134 	135 	136 	137 	138 	139 	140 	141 	142 	143
  line      1 	192 	193 	194 	195 	196 	197 	198 	199 	200 	201 	202 	203 	204 	205 	206 	207
  127 clear lcd

  LCD contrast circuit:
  R680 resistor from Arduino contrast pin to LCD contrast pin
  10uF cap from LCD contrast pin to gnd
*/

#include <LiquidCrystal.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
#define CONTR_PIN 10
#define CONTRAST 40 //adjust value for best contrast
#define LCD_LED_A_PIN 8
#define LCD_LED_K_PIN 9

char buffer[21];
char terminator = ';';
byte command;
byte cursorPos;
byte LCDline;

void setup() {
  // set pin for LCD contrast and LCD LED backlight (use resistor to limit current to max 20mAh):
  pinMode(CONTR_PIN, OUTPUT);
  analogWrite(CONTR_PIN, CONTRAST);
  pinMode(LCD_LED_A_PIN, OUTPUT);
  digitalWrite(LCD_LED_A_PIN, HIGH); 
  pinMode(LCD_LED_K_PIN, OUTPUT);
  digitalWrite(LCD_LED_K_PIN, LOW); 
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.clear();
  //Serial.print("Serial display");
  lcd.print("_");

  /*
    // set contrast
    while(1) {
    for (byte i = 0; i < 5; i++) {
    delay(1000);
    analogWrite(CONTR_PIN, i);
    lcd.setCursor(0, 0);
    lcd.print("Serial display ");
    lcd.print(i);
    Serial.println(i);
    }
    }
  */
}
void loop()
{
  if (Serial.available()) getSerial(); // get incoming serial
}

void getSerial() {
  Serial.readBytesUntil(terminator, buffer, 19);
  if ((buffer[0] & 0xFF) == 254) { //if valid special command 1st char
    command = buffer[1] & 0xFF;
    if (command >= 128 && command <= 143) {
      LCDprint(command - 128, 0);
    }
    else if (command >= 192 && command <= 207) {
      LCDprint(command - 192, 1);
    }
    else if (command == 127) {
      lcd.clear();
    }
  }
}

void LCDprint(byte _cursorpos, byte _lcdline) {
  lcd.setCursor(_cursorpos, _lcdline);
  for (byte a = (2 + _cursorpos); a < 18; a++) {
    lcd.write(buffer[a]);
  }
}


/*
   CG scale for F3F & F3B models
   Olav Kallhovd, 2016-2021
*/
#define VERSION "CG Scale SW v1.2.2"

#include "config.h"
#if defined USE_EEPROM
#if defined(ESP8266) || defined(AVR)
#include <EEPROM.h>
#endif
#else
#undef USE_EEPROM
#endif

//** eeprom address:
#define ADR_LDCELL1_CAL     0
#define ADR_LDCELL2_CAL     4
#define ADR_BATVOLT_CAL     8
#define ADR_LDCELL1_IS_CAL  12
#define ADR_LDCELL2_IS_CAL  13
#define ADR_BATVOLT_IS_CAL  14

#define IS_CAL_VAL          254

//** HX711 library:
#include <HX711_ADC.h> //https://github.com/olkal/HX711_ADC library can be installed from the library manager

//** HX711 constructors:
HX711_ADC LoadCell_1(LoadCell_1_DOUT_pin, LoadCell_1_SCK_pin);
HX711_ADC LoadCell_2(LoadCell_2_DOUT_pin, LoadCell_2_SCK_pin);

#define LCD_LEN 16

//** i2c LCD librarys and declaration:
#ifdef USE_I2CDISP
#include <Wire.h>
#include <hd44780.h> //https://github.com/duinoWitchery/hd44780 library can be installed from the library manager
#include <hd44780ioClass/hd44780_I2Cexp.h> 
hd44780_I2Cexp lcdI2C(I2CDISP_ADR,16,2); // declare lcd object 
#endif

byte seroutput = 0; //0: Serial LCD display, 1: Wt+CG+loadcell value, 2: other (calibration etc.)


//***
void setup() {
  Serial.begin(9600); //don't change if you use the serial LCD display option
  printMenu();
  setupLCD();
  setupLoadcells();

#ifdef USE_ZERO_BUTTON
  pinMode(zero_button_pin, INPUT_PULLUP); //optional led
#endif

#ifdef USE_LED
  pinMode(led_pin, OUTPUT); //optional led
  digitalWrite(led_pin, HIGH);
#endif
}


//***
void loop() {
  //** library function update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  //** longer delay in sketch will reduce effective sample rate (so be careful with use of delay())
  if (LoadCell_1.update() || LoadCell_2.update()) {
    displayWeightAndCG();
  }

  // receive command from serial terminal
  if (Serial.available() > 0) {
    handleIncSerial();
  }

  displayVoltage();
  zeroButton();
  flashLED();
}


//*** set up LCD:
void setupLCD() {
#ifdef USE_I2CDISP
  lcdI2C.begin(16, 2);
  lcdI2C.setBacklight(1);
#endif

  char lcdtext[LCD_LEN + 1] = {"F3X COG scale   "};
  printToLCD(lcdtext, 0);
  char lcdtext2[LCD_LEN + 1] = {"Wait...         "};
  printToLCD(lcdtext2, 1);
}


//*** set up loadcells and calibration values:
void setupLoadcells() {
  long stabilisingtime = 3000; // initial zero/tare precision can be improved by adding a few seconds of stabilising time in startup
  LoadCell_1.begin();
  LoadCell_2.begin();
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;
  while ((loadcell_1_rdy + loadcell_2_rdy) < 2) { //run startup, stabilisation and tare, both modules simultaneously
    if (!loadcell_1_rdy) loadcell_1_rdy = LoadCell_1.startMultiple(stabilisingtime);
    if (!loadcell_2_rdy) loadcell_2_rdy = LoadCell_2.startMultiple(stabilisingtime);
  }

#ifdef USE_EEPROM
  if (EEPROM.read(ADR_LDCELL1_IS_CAL) == IS_CAL_VAL) {
    EEPROM.get(ADR_LDCELL1_CAL, ldcell_1_calfactor);
  }
  if (EEPROM.read(ADR_LDCELL2_IS_CAL) == IS_CAL_VAL) {
    EEPROM.get(ADR_LDCELL2_CAL, ldcell_2_calfactor);
  }
  if (EEPROM.read(ADR_BATVOLT_IS_CAL) == IS_CAL_VAL) {
    EEPROM.get(ADR_BATVOLT_CAL, batvoltage_calfactor);
  }
#endif

  LoadCell_1.setCalFactor(ldcell_1_calfactor); // set calibration factor
  LoadCell_2.setCalFactor(ldcell_2_calfactor); // set calibration factor
}


//*** analog read and return battery voltage in mV units:
int readBattVoltage() {
  long battvalue = analogRead(batRef_pin); // analog reading
  battvalue *= batvoltage_calfactor;
  return battvalue;
}


//*** calculate model weight and CG and display the values:
void displayWeightAndCG() {
  //** calculate CG and update serial/LCD
  static long t1 = 0;
  const long CGoffset = ((WingPegDist / 2) + LEstopperDist) * 10;
  if (t1 < millis()) {
    t1 = millis() + lcdprintInterval;
    float a = LoadCell_1.getData();
    float b = LoadCell_2.getData();
    long weightAvr[3];
    float CGratio;
    long CG;
    weightAvr[0] = a * 100;
    weightAvr[1] = b * 100;
    long weightTot = weightAvr[0] + weightAvr[1];

    if (weightAvr[0] > 500 && weightAvr[1] > 500) {
      long a = weightAvr[1] / 10;
      long b = weightAvr[0] / 10;
      CGratio = (((a * 10000) / (a + b)));
      CG = ((((WingPegDist) * CGratio) / 1000) - ((WingPegDist * 10) / 2) + CGoffset);
    }
    else {
      CG = 0;
    }

    char lcdtext[LCD_LEN + 1] = "Wt:     CG:     ";
    if (weightTot < 0 && weightTot >= - 100) {
      weightTot = 0;
    }
    if (weightTot < -100) {
      lcdtext[3] = 'e';
      lcdtext[4] = 'r';
      lcdtext[5] = 'r';
      lcdtext[6] = '.';
    }
    else {
      lcdtext[3] = (char)(weightTot / 100000) + 48;
      lcdtext[4] = (char)((weightTot % 100000) / 10000) + 48;
      lcdtext[5] = (char)((weightTot % 10000) / 1000) + 48;
      lcdtext[6] = (char)((weightTot % 1000) / 100) + 48;
    }
    lcdtext[7] = ' ';
    if (CG != 0) {
      lcdtext[11] = (char)(CG / 10000) + 48;
      lcdtext[12] = (char)((CG % 10000) / 1000) + 48;
      lcdtext[13] = (char)((CG % 1000) / 100) + 48;
      lcdtext[14] = '.';
      lcdtext[15] = (char)((CG % 100) / 10) + 48;
    }

    printToLCD(lcdtext, 1);

    //**print values to serial terminal:
    static long t2 = 0;
    if (seroutput == 1) {
      if (t2 < millis()) {
        t2 = millis() + serialprintInterval;

        for (byte a = 0; a < 2; a++) {
          Serial.print(F("LdCell_"));
          Serial.print(a + 1);
          Serial.write(':');
          long i = weightAvr[a];
          if (i < 0) {
            Serial.write('-');
            i = ~weightAvr[a];
          }
          Serial.print(i / 100);
          Serial.write('.');
          if ((i % 100) < 10) {
            Serial.write('0');
          }
          Serial.print(i % 100);
          Serial.print(F("   "));
        }
        Serial.print(F("Wt:"));
        Serial.print((weightAvr[0] + weightAvr[1]) / 100);
        Serial.print(F("   CG:"));
        Serial.print(CG / 100);
        Serial.write('.');
        Serial.println(CG % 100);
      }
    }
  }
}


//*** handle battery voltage and display voltage on LCD:
void displayVoltage() {
  static long t = 0;
  if (millis() > t) {
    int batval = readBattVoltage();
    static int last_batval = -1;
    static bool show_value = false;
    static long lastbatwarn_time = 0;
    bool updatebatwarning = false;

    if (batval < lowBatVal) {
      show_value = !show_value;
      lastbatwarn_time = millis();
      updatebatwarning = true;
    }
    else {
      if (!show_value) updatebatwarning = true;
      show_value = true;
    }

    if ((last_batval != batval) || updatebatwarning) {
      char lcdtext[LCD_LEN + 1];
      memset(lcdtext, 32, sizeof(lcdtext));
      if (show_value) {
        lcdtext[0] = (char)(batval / 1000) + 48;
        lcdtext[1] = '.';
        lcdtext[2] = (char)((batval % 1000) / 100) + 48;
        //lcdtext[3] = (char)((batval % 100) / 10) + 48;
        lcdtext[3] = 'V';
      }
      printToLCD(lcdtext, 0);
      last_batval = batval;
    }
    t = millis() + 750;
  }
}

//*** print char array to LCD:
void printToLCD(char *lcdtext, byte lineno) {
#ifdef USE_SERDISP
  if (seroutput == 0) {
    Serial.write(254);
    Serial.write(128 + (lineno * 64)); //pos line 0, char 0
    for (byte a = 0; a < LCD_LEN; a++) {
      Serial.write(lcdtext[a]);
    }
    Serial.write(';'); //end
  }
#endif

#ifdef USE_I2CDISP
  lcdI2C.setCursor(0, lineno); //pos line 0, char 0
  for (byte a = 0; a < LCD_LEN; a++) {
    lcdI2C.write(lcdtext[a]);
  }
#endif
}


//*** print serial input menu:
void printMenu() {
  Serial.println();
  Serial.println(F(VERSION));
  Serial.println(F("Menu:"));
  Serial.println(F("Send 's' to enable weight/CG output to Serial Monitor"));
  Serial.println(F("Send 'z' to set zero offset"));
  Serial.println(F("Send 'c1' to start FRONT load cell calibration"));
  Serial.println(F("Send 'c2' to start REAR load cell calibration"));
  Serial.println(F("Send 'b' to start battery voltage calibration"));
  Serial.println(F("Send 'm1' to manually set FRONT load cell calibration value"));
  Serial.println(F("Send 'm2' to manually set REAR load cell calibration value"));
}


//*** handle incoming commands from serial monitor:
void handleIncSerial() {
  char buf[3];
  memset(buf, 0, sizeof(buf));
  Serial.readBytes(buf, 2);

  if ((buf[0] & 0xFF) == 'z') {
    setZero(); //set zero
  }
  else if ((buf[0] & 0xFF) == 's') {
    //Serial.println(F("print Wt+CG+Loadcell to Serial"));
    seroutput = 1;
  }
  else if ((buf[0] & 0xFF) == 'c') { //if valid command
    if ((buf[1] & 0xFF) == '1') {
      //Serial.println(F("calibrate1"));
      calibrateLoadcell(1); //calibrate loadcell front
    }
    else if ((buf[1] & 0xFF) == '2') {
      //Serial.println(F("calibrate2"));
      calibrateLoadcell(2); //calibrate loadcell rear
    }
  }
  else if ((buf[0] & 0xFF) == 'm') { //if valid command
    if ((buf[1] & 0xFF) == '1') {
      //Serial.println(F("changeSavedCalFactor1"));
      setLoadcellCalValue(1); //set calibration value for loadcell front manually
    }
    else if ((buf[1] & 0xFF) == '2') {
      //Serial.println(F("changeSavedCalFactor2"));
      setLoadcellCalValue(2); //set calibration value for loadcell rear manually
    }
  }
  else if ((buf[0] & 0xFF) == 'b') { //if valid command
    calibrateBatVoltage();
  }
}


//*** set zero offset, both loadcells simultaneously:
void setZero() {
  char lcdtext[LCD_LEN + 1] = {"Setting Zero... "};
  printToLCD(lcdtext, 1);
  boolean _resume = false;
  boolean zero_1 = false;
  boolean zero_2 = false;
  LoadCell_1.tareNoDelay();
  LoadCell_2.tareNoDelay();
  while (_resume == false) {
    LoadCell_1.update();
    LoadCell_2.update();
    if (LoadCell_1.getTareStatus() == true) zero_1 = true;
    if (LoadCell_2.getTareStatus() == true) zero_2 = true;
    if (zero_1 && zero_2) {
      if (seroutput == 1) {
        Serial.println(F("Set zero offset complete "));
      }
      _resume = true;
    }
  }
}


//*** calibrate loadcell, input '1' is front, '2' is rear:
void calibrateLoadcell(byte loadcellno) {
  if ((loadcellno != 1) && (loadcellno != 2)) return;
  int eepromAdr;
  int eepromAdrIsCalibrated;
  float newCalibrationValue;
  char lcdtext[LCD_LEN + 1] = {"Calibration...  "};
  printToLCD(lcdtext, 1);
  Serial.println(F("***"));
  Serial.print(F("Start calibration loadcell "));
  if (loadcellno == 1) Serial.println(F("FRONT:"));
  else Serial.println(F("REAR:"));
  Serial.println(F("Place the CG scale an a level stable surface."));
  Serial.println(F("Send 'z' from serial monitor to set the zero offset."));

  boolean _resume = false;
  boolean zero_1 = false;
  boolean zero_2 = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'z') {
        setZero();
        _resume = true;
        printToLCD(lcdtext, 1);
      }
    }
  }

  Serial.print(F("Now, place your known mass on the loadcell "));
  if (loadcellno == 1) Serial.print(F("FRONT, "));
  else Serial.print(F("REAR, "));
  Serial.println(F("then send the weight of this mass (i.e. 500.0) from the serial monitor."));
  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell_1.update();
    LoadCell_2.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print(F("Known mass is: "));
        Serial.println(known_mass);
        _resume = true;
      }
      else {
        Serial.println(F("Invalid value"));
      }
    }
  }

  if (loadcellno == 1) {
    LoadCell_1.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
    newCalibrationValue = LoadCell_1.getNewCalibration(known_mass); //get the new calibration value
#ifdef USE_EEPROM
    eepromAdr = ADR_LDCELL1_CAL;
    eepromAdrIsCalibrated = ADR_LDCELL1_IS_CAL;
#endif
  }

  if (loadcellno == 2) {
    LoadCell_2.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
    newCalibrationValue = LoadCell_2.getNewCalibration(known_mass); //get the new calibration value
#ifdef USE_EEPROM
    eepromAdr = ADR_LDCELL2_CAL;
    eepromAdrIsCalibrated = ADR_LDCELL2_IS_CAL;
#endif
  }

  Serial.print(F("Calculated calibration value is: "));
  Serial.print(newCalibrationValue);
  Serial.println(F(", use this in your project sketch or save to eeprom."));
  
#ifdef USE_EEPROM
  Serial.print(F("Save this value to EEPROM adress "));
  Serial.print(eepromAdr);
  Serial.println(F("? y/n"));
  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
        EEPROM.put(eepromAdr, newCalibrationValue);
        EEPROM.write(eepromAdrIsCalibrated, IS_CAL_VAL);
#if defined(ESP8266)
        EEPROM.commit();
#endif
        EEPROM.get(eepromAdr, newCalibrationValue);
        Serial.print(F("Value "));
        Serial.print(newCalibrationValue);
        Serial.print(F(" saved to EEPROM address: "));
        Serial.println(eepromAdr);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println(F("Value not saved to EEPROM"));
        _resume = true;
      }
    }
  }
#endif
  Serial.println(F("End calibration"));
  Serial.println(F("***"));
}

//*** enter loadcell calibration manually, input '1' is front, '2' is rear:
void setLoadcellCalValue(byte loadcellno) {
  if ((loadcellno != 1) && (loadcellno != 2)) return;
  int eepromAdr;
  int eepromAdrIsCalibrated;
  float c = 0;
  boolean _resume = false;

  Serial.println(F("***"));
  Serial.print(F("Current value for loadcell "));
  if (loadcellno == 1) {
    c = LoadCell_1.getCalFactor();
    Serial.print(F("FRONT is:"));
#ifdef USE_EEPROM
    eepromAdr = ADR_LDCELL1_CAL;
    eepromAdrIsCalibrated = ADR_LDCELL1_IS_CAL;
#endif
  }
  else {
    c = LoadCell_2.getCalFactor();
    Serial.print(F("REAR is:"));
#ifdef USE_EEPROM
    eepromAdr = ADR_LDCELL2_CAL;
    eepromAdrIsCalibrated = ADR_LDCELL2_IS_CAL;
#endif
  }
  Serial.println(c);
  Serial.println(F("Now, send the new value from serial monitor, i.e. 696.0"));
  while (_resume == false) {
    if (Serial.available() > 0) {
      c = Serial.parseFloat();
      if (c != 0) {
        Serial.print(F("Calibration value is: "));
        Serial.println(c);
        if (loadcellno == 1) LoadCell_1.setCalFactor(c);
        else LoadCell_2.setCalFactor(c);
        _resume = true;
      }
      else {
        Serial.println(F("Invalid value, exit"));
        return;
      }
    }
  }

#ifdef USE_EEPROM
  Serial.print(F("Save this value to EEPROM adress "));
  Serial.print(eepromAdr);
  Serial.println(F("? y/n"));
  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
        EEPROM.put(eepromAdr, c);
        EEPROM.write(eepromAdrIsCalibrated, IS_CAL_VAL);
#if defined(ESP8266)
        EEPROM.commit();
#endif
        EEPROM.get(eepromAdr, c);
        Serial.print(F("Value "));
        Serial.print(c);
        Serial.print(F(" saved to EEPROM address: "));
        Serial.println(eepromAdr);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println(F("Value not saved to EEPROM"));
        _resume = true;
      }
    }
  }
#endif
  Serial.println(F("End change calibration value"));
  Serial.println(F("***"));
}


//*** calibrate battery voltage measurement:
void calibrateBatVoltage() {
  int analogreadval = 0;
  float voltage, calibrationval;
  boolean _resume = false;
  Serial.println(F("***"));
  Serial.println(F("Start calibration battery voltage (be sure that the battery is connected):"));
  Serial.print(F("Average analog read value from batRef_pin is:"));
  for (byte i = 0; i < 10; i++) {
    analogreadval += analogRead(batRef_pin);
    delay(10);
  }
  analogreadval /= 10;
  Serial.println(analogreadval);
  if ((analogreadval == 0) || (analogreadval == 1023)) {
    Serial.println(F("Analog read value is to high or to low, check your circuit, exit"));
    return;
  }
  Serial.println(F("Measure the voltage at the MCU VIN pin using a multimeter."));
  Serial.println(F("Send the result (i.e. 8.65) from the serial monitor."));
  while (_resume == false) {
    if (Serial.available() > 0) {
      voltage = Serial.parseFloat() * 1000;
      if (voltage != 0) {
        calibrationval = voltage / analogreadval;
        Serial.print(F("Calculated calibration value is: "));
        Serial.println(calibrationval);
        batvoltage_calfactor = calibrationval;
        _resume = true;
      }
      else {
        Serial.println(F("Invalid value, exit"));
        return;
      }
    }
  }

#ifdef USE_EEPROM
  Serial.print(F("Save this value to EEPROM adress "));
  Serial.print(ADR_BATVOLT_CAL);
  Serial.println(F("? y/n"));
  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
        EEPROM.put(ADR_BATVOLT_CAL, calibrationval);
        EEPROM.write(ADR_BATVOLT_IS_CAL, IS_CAL_VAL);
#if defined(ESP8266)
        EEPROM.commit();
#endif
        EEPROM.get(ADR_BATVOLT_CAL, calibrationval);
        Serial.print(F("Value "));
        Serial.print(calibrationval);
        Serial.print(F(" saved to EEPROM address: "));
        Serial.println(ADR_BATVOLT_CAL);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println(F("Value not saved to EEPROM"));
        _resume = true;
      }
    }
  }
#endif
  Serial.println(F("End calibration voltage"));
  Serial.println(F("***"));
}


//*** handle zero offset button:
void zeroButton() {
#ifdef USE_ZERO_BUTTON
  static long lastSwTime = 0;
  static bool lastSwState = 1;
  const int debounceValue = 50;
  bool swstate = digitalRead(zero_button_pin);
  if (swstate != lastSwState) {
    if ((lastSwTime + debounceValue) < millis()) {
      if (swstate == HIGH) { //button was released
        delay(250);
        setZero();
      }
      lastSwState = swstate;
      lastSwTime = millis();
    }
  }
#endif
}


//*** flash power-on LED:
void flashLED() {
#ifdef USE_LED
  static long t = 0;
  static boolean ledState = 0;
  if (t < millis()) {
    if (ledState) {
      t = millis() + 2000;
      ledState = 0;
    }
    else {
      t = millis() + 100;
      ledState = 1;
    }
    digitalWrite(led_pin, ledState);
  }
#endif
}

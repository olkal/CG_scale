/*
   CG scale for F3F & F3B models
   Olav Kallhovd 2016-2017

   CG Scale main components:
   1 pc load sensor front YZC-133 2kg
   1 pc load sensor rear YZC-133 3kg
   2 pc HX711 ADC, one for each load sensor (128bit resolution)
   1 pc Arduino Pro for the scale
   1 pc Arduino Pro for the Serial display
   1 pc 16*2 HD44780 LCD for the Serial display
   3D printed parts

   Max model weight with sensors above: 4 to 4,5kg depending on CG location

*/

#include <HX711_ADC.h> //https://github.com/olkal/HX711_ADC can be installed from the library manager
//Number of samples and some filtering settings can be adjusted in the HX711_ADC.h library file
//The best RATE setting is usually 10SPS, see HX711 data sheet (HX711 pin 15, can usually be set by a solder jumper on the HX711 module)
//RATE 80SPS will also work fine, but conversions will be more noisy, so consider increasing number of samples in HX711_ADC.h

//HX711 constructor (dout pin, sck pint):
HX711_ADC LoadCell_1(A2, A3); //HX711 pins front sensor (DOUT, PD_SCK)
HX711_ADC LoadCell_2(A0, A1); //HX711 pins rear sensor (DOUT, PD_SCK)


// I²C-Display, if available
// Enable I²C-Display?
// #define I2CDISP
#ifdef I2CDISP
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F or 0x27 for a 16 chars and 2 line display
#endif

byte ledPin = 3;
byte batRefPin = A4;
char toLCD[20];
byte output;
boolean ledState;
long t1;
long t2;

const int printInterval = 500; // LCD/Serial refresh interval

//*** configuration:
//*** set dimensional calibration values:
const long WingPegDist = 1198; //calibration value in 1/10mm, projected distance between wing support points, measure with calliper
const long LEstopperDist = 300; //calibration value 1/10mm, projected distance from front wing support point to leading edge (stopper pin), measure with calliper
//*** set scale calibration values (best to have the battery connected when doing calibration):
const float ldcell_1_calfactor = 954.0; // user set calibration factor load cell front (float)
const float ldcell_2_calfactor = 799.0; // user set calibration factor load cell rear (float)
//***
const long stabilisingtime = 3000; // tare precision can be improved by adding a few seconds of stabilising time
//***
const long CGoffset = ((WingPegDist / 2) + LEstopperDist) * 10;

void setup() {
  //***
  // 0: Serial-Console (Your PC); 
  // 1: Serial-LCD (2nd Arduino with SimpleSerialDisplay)
  // 2: I2C-LCD (define I2CDISP above!)
  output = 0;
  //***
  
  Serial.begin(9600);
  Serial.write(254);
  Serial.write(128);
  Serial.print("F3X COG scale   ;");
  Serial.write(254);
  Serial.write(192);
  Serial.print("Bat:");
  int batval = readBattVoltage();
  Serial.write((char)(batval / 1000) + 48);
  Serial.print(".");
  Serial.write((char)((batval % 1000) / 100) + 48);
  Serial.write((char)((batval % 100) / 10) + 48);
  Serial.print("V     ");
  if (output == 0) { //if output to serial terminal
    Serial.println();
    Serial.println("Wait for stabilising and tare...");
  }
#ifdef I2CDISP
  if (output != 0)
    Serial.println("You have I2CDISP defined, but output is not 2. Are you sure thats right?");

  Serial.println("Initializing I2C-Display");
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("F3X COG scale,");
  lcd.setCursor(0, 1);
  lcd.print("at your service!");
#endif


  LoadCell_1.begin();
  LoadCell_2.begin();
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;
  while ((loadcell_1_rdy + loadcell_2_rdy) < 2) { //run startup, stabilisation and tare, both modules simultaneously
    if (!loadcell_1_rdy) loadcell_1_rdy = LoadCell_1.startMultiple(stabilisingtime);
    if (!loadcell_2_rdy) loadcell_2_rdy = LoadCell_2.startMultiple(stabilisingtime);
  }
  LoadCell_1.setCalFactor(ldcell_1_calfactor); // set calibration factor
  LoadCell_2.setCalFactor(ldcell_2_calfactor); // set calibration factor

  pinMode(ledPin, OUTPUT); //led
  digitalWrite(ledPin, HIGH);

  if (output == 1) { //if output to LCD
    Serial.write(254);
    Serial.write(127); // clear lcd
  }
}

int readBattVoltage() { // read battery voltage
  long battvalue = 0;
  battvalue += analogRead(batRefPin);
  battvalue += analogRead(batRefPin);
  battvalue *= 4883L; // analog reading * (5.00V*1000000)/1024 (adjust value if VCC is not 5.0V)
  battvalue /= 640L; // this number comes from the resistor divider value ((R2/(R1+R2))*1000)/noof analogreadings (adjust value if required)
  //Serial.println(battvalue);
  if (battvalue < 7500) { //
    //low bat warning code goes here, not implemented
  }
  return battvalue;
}

void flashLED() {
  if (t2 < millis()) {
    if (ledState) {
      t2 = millis() + 2000;
      ledState = 0;
    }
    else {
      t2 = millis() + 100;
      ledState = 1;
    }
    digitalWrite(ledPin, ledState);
  }
}

void loop() {
  //library function update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  //longer delay in scetch will reduce effective sample rate (be careful with delay() in loop)
  LoadCell_1.update();
  LoadCell_2.update();

  // calculate CG and update serial/LCD
  if (t1 < millis()) {
    t1 = millis() + printInterval;
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
    // if output = 0: print result to serial terminal:
    if (output == 0) {
      for (byte a = 0; a < 2; a++) {
        Serial.print("weight_LdCell_");
        Serial.print(a + 1);
        Serial.print(": ");
        long i = weightAvr[a];
        if (i < 0) {
          Serial.print('-');
          i = ~weightAvr[a];
        }
        Serial.print(i / 100);
        Serial.print('.');
        if ((i % 100) < 10) {
          Serial.print("0");
        }
        Serial.print(i % 100);
        Serial.print("      ");
      }
      Serial.print("CG:");
      Serial.print(CG / 100);
      Serial.print('.');
      Serial.println(CG % 100);
    } else if (output == 1) { //if output = 1: print to serial LCD
      toLCD[0] = 254;
      toLCD[1] = 192;
      toLCD[2] = 'W';
      toLCD[3] = 't';
      toLCD[4] = ':';
      if (weightTot < 0 && weightTot >= - 100) {
        weightTot = 0;
      }
      if (weightTot < -100) {
        toLCD[5] = 'E';
        toLCD[6] = 'r';
        toLCD[7] = 'r';
        toLCD[8] = '.';
      }
      else {
        toLCD[5] = (char)(weightTot / 100000) + 48;
        toLCD[6] = (char)((weightTot % 100000) / 10000) + 48;
        toLCD[7] = (char)((weightTot % 10000) / 1000) + 48;
        toLCD[8] = (char)((weightTot % 1000) / 100) + 48;
      }
      toLCD[9] = ' ';
      if (CG != 0) {
        toLCD[10] = 'C';
        toLCD[11] = 'G';
        toLCD[12] = ':';
        toLCD[13] = (char)(CG / 10000) + 48;
        toLCD[14] = (char)((CG % 10000) / 1000) + 48;
        toLCD[15] = (char)((CG % 1000) / 100) + 48;
        toLCD[16] = '.';
        toLCD[17] = (char)((CG % 100) / 10) + 48;
      }
      else {
        for (byte s = 10; s < 18; s++) {
          toLCD[s] = ' ';
        }
      }
      toLCD[18] = ';';
      for (byte i = 0; i < sizeof(toLCD) - 1; i++) {
        Serial.write(toLCD[i]);
      }
    } else if (output == 2) { //if output = 2: print to I2C LCD
      #ifdef I2CDISP
      lcd.clear();
      // 1st Line: Weight
      lcd.setCursor(0, 0);
      if (weightTot < 0 && weightTot >= - 100)
        weightTot = 0;
      if (weightTot < -100)
        lcd.print("Wt: Error!");
      else {
        sprintf(toLCD, "Wt: %ug", weightTot/100);
        lcd.print(toLCD);
      }
      // 2nd Line: CG
      lcd.setCursor(0, 1);
      if (CG == 0)
        lcd.print("CG: Error!");
      else {
        sprintf(toLCD, "CG: %ld.%ldmm", CG/100, CG%100);
        lcd.print(toLCD);
      }
      #endif
    }
    readBattVoltage();
  }
  flashLED();
}

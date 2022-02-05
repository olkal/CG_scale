
//** HX711 library configuration:
//** Number of samples and some filtering settings can be adjusted in the HX711_ADC.h library file
//** The best RATE setting is usually "LOW" 10SPS, see HX711 data sheet (RATE is selected by HX711 pin 15, this can be set by a solder jumper on some HX711 modules)
//** RATE "HIGH" 80SPS will also work, but conversions will be more noisy, so consider increasing number of samples in HX711_ADC.h

//** If you are setting up a new scale, please consider first testing the indivdual loadcells using the example file provided with the HX711 library, "Read_1x_load_cell.ino",
//** to ensure that direction of up/down force is working as it should. If not correct the loadcell wiring.



//** Uncomment one of the two lines below to enable either serial 16x2 character display or i2c 16x2 character display:
//#define USE_SERDISP
#define USE_I2CDISP

//** If using i2c display, set correct i2c address (if you don't know, use the i2c Scanner sketch: https://playground.arduino.cc/Main/I2cScanner/):
#define I2CDISP_ADR 0x20

//** Uncomment to enable optional EEPROM storing of loadcell calibration value (ldcell_1_calfactor and ldcell_2_calfactor):
#define USE_EEPROM

//** Uncomment if you want to connect an optional push button button to manually set zero offset
//#define USE_ZERO_BUTTON

//** Uncomment  if you want to connect an optional blinking LED for indicating power on (it's not very useful...)
//#define USE_LED

//** Set low battery voltage in units mV, battery voltage shown on the LCD will blink if voltage is below this value
const int lowBatVal = 6000;

//** Set the dimensional calibration values (see instruction sheet) in unit 1/10mm, measure using a calliper:
const long WingPegDist = 1200; //projected distance between wing support points
const long LEstopperDist = 300; //projected distance from front wing support point to leading edge (stopper pin)

//** Set loadcell calibration values (best to have the battery connected when doing calibration).
//** You can get the correct values by running the calibration routine and then insert the values in the sketch before re-uploading.
//** If you have enabled EEPROM (#define USE_EEPROM) the values below will be ignored once you have run the calibration routine and stored the values in EEPROM.
float ldcell_1_calfactor = 954.0; // calibration factor load cell front
float ldcell_2_calfactor = 799.0; // calibration factor load cell rear

//** LCD refresh interval ms
const int lcdprintInterval = 100;

//** serial monitor refresh interval ms
const int serialprintInterval = 500;

//** Battery voltage calibration:
//** The calibration value below comes from the VCC (5000mV), the analog resolution (1024) and the resistor divider circuit, like this:
//** ((5000*1000L)/1024) / ((R2/(R1+R2))*1000). This calibration value is multiplied with the analog read value to get the voltage in mV.
//** You can get the correct value by running the calibration routine and then insert the value in the sketch before re-upload.
//** If you have enabled EEPROM (#define USE_EEPROM) the value below will be ignored once you have run the calibration routine and stored the value in EEPROM.
float batvoltage_calfactor = 15.26;

//** Pin configuration for Arduino Pro-Mini and Arduino Nano:
//** If you use an i2c display, pin A4 is SDA and pin A5 is SCL for the display communication
//** If you use the Serial display, pin 1 for the display serial communication
//** Note that on the Atmega328 pin A6 and A7 is for analog read only.
const byte LoadCell_1_DOUT_pin = A2;    //HX711 DOUT pin for load cell front
const byte LoadCell_1_SCK_pin = A3;     //HX711 SCK pin for load cell front
const byte LoadCell_2_DOUT_pin = A0;    //HX711 DOUT pin for load cell rear
const byte LoadCell_2_SCK_pin = A1;     //HX711 SCK pin for load cell rear
const byte batRef_pin = A6;             //Battery voltage sense via resistor divider, range 0-5v (0-VCC)
const byte zero_button_pin = 5;         //optional zero offset push button
const byte led_pin = 3;                 //optional power-on blinking LED

//** Pin configuration for ESP8266, TODO!:
//const byte LoadCell_1_DOUT_pin = ;    //HX711 DOUT pin for load cell front
//const byte LoadCell_1_SCK_pin = ;     //HX711 SCK pin for load cell front
//const byte LoadCell_2_DOUT_pin = ;    //HX711 DOUT pin for load cell rear
//const byte LoadCell_2_SCK_pin = ;     //HX711 SCK pin for load cell rear
//const byte batRef_pin = A0;           //Battery voltage sense via resistor divider, range 0-1v
//const byte led_pin = ;                //optional power on blink LED
//const byte zero_button_pin = ;        //optional zero offset push button

//** you may define i2c LCD custom pins for non-standard LCD backpack circuit (optional). 
//** Uncomment if you want to define custom pins, and then define pins as you wish:
//#define USE_CUSTOM_I2C_PINS
//** LCD backpack internal pin mapping:
#define BACKLIGHT_PIN  7
#define En_pin  4
#define Rw_pin  5
#define Rs_pin  6
#define D4_pin  0
#define D5_pin  1
#define D6_pin  2
#define D7_pin  3

/*
 * CG scale for F3F & F3B models
 * Olav Kallhovd 2016
 * 
   CG Scale main components:
   1 pc load sensor front YZC-133 2kg
   1 pc load sensor rear YZC-133 3kg
   2 pc HX711 ADC, one for each load sensor (128bit resolution)
   1 pc Arduino Pro
   1 pc Serial display (16*2 LCD w/ Arduino Pro)

   Max model weight with suggested sensors: 4 to 4,5kg depending on CG location
*/

#include "HX711.h" //https://github.com/bogde/HX711

HX711 scale_1(A2, A3);//HX711 pins front sensor (DOUT, PD_SCK)
HX711 scale_2(A0, A1);//HX711 pins rear sensor (DOUT, PD_SCK)

byte ledPin = 3;
byte batRefPin = A4;

long battValue;
long weightAvr[3];
long weightSng[3];
long weightTot;
float CGratio;
long CG;
long WingPegDist = 1198; //calibration value in 1/10mm, projected distance between wing support points, measure with calliper
long LEstopperDist = 300; //calibration value 1/10mm, projected distance from front wing support point to leading edge (stopper pin), measure with calliper
long CGoffset = ((WingPegDist / 2) + LEstopperDist) * 10;;
char toLCD[20];
boolean output;
long t;

void setup() {
  Serial.begin(9600);
  delay(1000); //stabilize
  int sens_cal_1 = 954;// this value is obtained by calibrating the front sensor with known weights
  int sens_cal_2 = 799; // this value is obtained by calibrating the rear sensor with known weights
  scale_1.set_scale(sens_cal_1); 
  scale_2.tare(30); // reset the scale to 0
  scale_2.set_scale(sens_cal_2);
  scale_1.tare(30); // reset the scale to 0 

  pinMode(ledPin, OUTPUT); //led
  digitalWrite(ledPin, HIGH);

  output = 1; // 0:Serial terminal (for calibrating), 1:LCD
  int batval = readBattVoltage();
  Serial.write(254);
  Serial.write(128);
  Serial.print("F3X COG scale   ;");
  Serial.write(254);
  Serial.write(192);
  Serial.print("Bat:");
  Serial.write((char)(batval / 1000) + 48);
  Serial.print(".");
  Serial.write((char)((batval % 1000) / 100) + 48);
  Serial.write((char)((batval % 100) / 10) + 48);
  Serial.print("V     ");
  delay(3000);
  Serial.write(254);
  Serial.write(127); // clear lcd

}
int readBattVoltage() { // read battery voltage
  long newbattvalue = 0;
  for (byte a = 0; a < 5; a++) {
    newbattvalue += analogRead(batRefPin);
  }
  newbattvalue /= 5;
  newbattvalue *= 502L; // vout=(reading * (5.00V*10000)/1023 (calculated this way in order to avoid use of floats) calibrate if required
  newbattvalue /= 32;
  //Serial.println(newbattvalue);
  if ((newbattvalue < 7500) && (battValue < 7500)) { //
    //low bat warning, not implemented
  }
  battValue = newbattvalue;
  return battValue;
}

void loop() {
  if (t < millis()) {
    t = millis() + 1000;
    //read sensors:
    scale_1.power_up();
    scale_2.power_up();
    float a = scale_1.get_units(20);
    weightAvr[0] = a * 100;
    float b = (scale_2.get_units(20));
    weightAvr[1] = b * 100;
    weightTot = weightAvr[0] + weightAvr[1];
    scale_1.power_down(); // put the ADC in sleep mode
    scale_2.power_down(); // put the ADC in sleep mode

    if (weightAvr[0] > 500 && weightAvr[1] > 500) {
      long a = weightAvr[1] / 10;
      long b = weightAvr[0] / 10;
      CGratio = (((a * 10000) / (a + b))); //*1246);// - ((WingPegDist/2))); //+356
      float _CG = ((((WingPegDist) * CGratio) / 1000) - ((WingPegDist * 10) / 2) + CGoffset);
      CG = _CG;
    }
    else {
      CG = 0;
    }

    if (!output) { //serial
      for (byte a = 0; a < 2; a++) {
        Serial.print("\t| average_");
        Serial.print(a + 1);
        Serial.print(":\t");
        if(weightAvr[a] < 0) {
          Serial.print('-');
          weightAvr[a] = ~weightAvr[a];
        }
        Serial.print(weightAvr[a] / 100);
        Serial.print('.');
        if((weightAvr[a] % 100) < 10) {
          Serial.print("0");
        }
        Serial.println(weightAvr[a] % 100);
        Serial.print("CGratio:");
        Serial.print(CGratio);
        Serial.print("  CG:");
        Serial.print(CG / 100);
        Serial.print(';');
        Serial.println(CG % 100);
      }
    }
    else { //serial to LCD
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
    }
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
  }
}


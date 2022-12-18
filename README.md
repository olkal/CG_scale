# F3X CG_scale
Arduino based Open Source CG scale for F3X gliders (and other model airplanes)
The scale can be used for most modern F3F/F3B gliders with slim fuselages and will calculate the CG and weight.


![CG_scale](https://github.com/olkal/CG_scale/blob/master/Documentation/small_picture.png?raw=true)

Update 18.12.2022 1.2.2, changed to a better i2c library for the 1602 LCD

Update 05.02.2022 1.2.1, minor bug fix + optional larger LCD box bottom part

Update 01.12.2021 1.2.0, Arduino code re-write:
- New function: Serial Monitor menu/input
- New function: Calibration routine for loadcells
- New function: Calibration routine for battery voltage
- Configuration parameters in new file config.h
- Store calibration values in EEPROM
- I2C Display code changed
- Display battery voltage also for I2C display
- Optional Zero Offset button included
- Low battery warning
- Updated instructions, circuit diagram and BOM
- ++


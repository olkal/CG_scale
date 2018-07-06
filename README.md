# F3X CG_scale
Arduino based Open Source CG scale for F3X gliders (and other model airplanes)
The scale can be used for most modern F3F/F3B gliders with slim fuselages and will calculate the CG and weight.


![CG_scale](https://github.com/olkal/CG_scale/blob/master/Documentation/small_picture.png?raw=true)

Update 05.01.2018:
STEP file has been included in the Documentation folder. This is not the original cad file, but it should be possible to import this universal format into most 3d cad software.

Update 14.10.2017:
CG_scale.ino - a new HX711 library has been implemented: https://github.com/olkal/HX711_ADC 
The library can be installed from the Arduino Library Manager.
We can now poll the HX711 for the next ready conversion rather than waiting for a series conversions to complete. The result is that the 10SPS rate setting and/or a higher number of samples can be used without unaccaptable processing lag, and the two load cells will now do conversions simultaneously rather than one by one.
The data filtering has also been improved.

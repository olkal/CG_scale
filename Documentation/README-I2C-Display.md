The I²C-Display is rather easy to connect:
- Connect Display GND to any Arduino-GND that is free
- Connect Display VCC to Arduino VCC
- Connect Display SDA to Arduino A4
- Connect Display SCL to Arduino A5
- If you want to retain battery-voltage monitoring, connect it to any other ADC-Pin, like A6 or A7 — and change `byte batRefPin = A?;` accordingly

To use, you need to:
- uncomment `#define I2CDISP`
- set `output = 2`
- Install a working LiquidCrystal_I2C-Library, I used: [This](https://github.com/DFRobot/WikiResource/blob/master/DFR0063/LiquidCrystal_I2C.zip)

The displays supported by this are kind of ubiquitous — many 1602-LCDs now come with a I²C-Portexpander piggybacked on the back. It even has a nice contrast adjustment, and jumper for the backlight! I used this one from BangGood:
[BangGood](https://www.banggood.com/IIC-I2C-1602-Blue-Backlight-LCD-Display-Module-For-Arduino-p-950726.html)
However, googling or asking ebay/aliexpress/amazon/whatever for »I2C 1602 LCD« will turn up many results.

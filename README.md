# SvDisplay
It is my small project to add Gead display functionality to Suzuki SV650 K3+.<br>
### All functionality:<br>
- Gear display (voltage from pink cable)<br>
- Bluetooth logging and configuration by dedicated Android application (optional)<br>
- Temerature and Humidity measurement and display by additional module (optional)<br>
- Battery voltage measurement by additional module (optional)<br>

### Hardware specification:<br>
- Main MPU - Arduino Nano v3<br>
![](http://i1374.photobucket.com/albums/ag438/bula87/nanov3_zpsxaan45q4.jpg)<br>
- Display - 128X64 OLED I2C 0.96"<br>
![](http://i1374.photobucket.com/albums/ag438/bula87/OLED_zpsywgyqafy.jpg)<br>
- Bluetooth module - HC-05 (optional)<br>
![](http://i1374.photobucket.com/albums/ag438/bula87/hc-05_zpsehq8frkw.png)<br>
- DHT11/DHT22(not tested yet) module (optional)<br>
![](http://i1374.photobucket.com/albums/ag438/bula87/DHT11_zpspqgoc1rf.jpg)<br>
- Standard Voltage DC 0-25V Sensor Module (optional)<br>
![](http://i1374.photobucket.com/albums/ag438/bula87/voltagesensor_zpswzwnpskc.jpg)<br>

### Connection specification:<br>
![Connection specification](http://i1374.photobucket.com/albums/ag438/bula87/Untitled%20Sketch_bb_zpspygligui.png)<br>
(illustrative photo - only connections are important)

### Needed libs:
DHT-sensor lib: [](https://github.com/adafruit/DHT-sensor-library)<br>
EEPROMEx lib: [](https://github.com/thijse/Arduino-EEPROMEx)<br>
Timer lib: [](https://github.com/JChristensen/Timer)<br>
SvOLED lib: included in Resources/Libs directory<br>

### Additional software:<br>
LCD graphic converter: http://www.hobbytronics.co.uk/downloads/bmp-lcd




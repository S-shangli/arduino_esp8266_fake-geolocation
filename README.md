# arduino_esp8266_fake-geolocation
Fake-GeoLocation for Arduino (ESP8266)

## references
* markszabo/FakeBeaconESP8266 https://github.com/markszabo/FakeBeaconESP8266

## hardware
* ESP8266 (I use ESP-WROOM-02)
* any USB to Serial converter (I use FT232XX)

## software
* Arduino IDE https://www.arduino.cc/
* ESP8266 core for Arduino https://github.com/esp8266/Arduino

# how it work
* Send wifi beacon.
* SSID, MAC address, transmitt power, wifi channnel and beacon interval can be changed.
* Multiple settings can be used simultaneously

## settings
|item|value(unit)|description|
|:---:|:---:|:---:|
|beacon_interval_ms|100(ms)|beacon interval|
|wifissid|"SSID_string"|SSID name|
|macaddr|{0x12,0x34,0x56,0x78,0x9a,0xbc}|MAC address|
|txpwr|100.0(%)|transmitt power(%)|
|channel|12(channel)|use channel|

## works
<img alt="screenshot" src="https://raw.githubusercontent.com/S-shangli/arduino_esp8266_fake-geolocation/master/screenshot01.png" width="300px"/>

# disclaimer
* Use at your own risk.
* It is often necessary to use it in a shield room.
* for development, education, hobby.

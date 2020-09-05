# EmlaLockSafe
This project is about controlling a cheap digital safe from a local hardware store using the API of emlalock https://www.emlalock.com. The safe I've got is the following: https://www.bauhaus.info/tresore/moebeleinsatztresor-security-box-mini/p/23519628

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/safe.png" width="100%">

# Contents
- [Understanding the locking mechanism](#understanding-the-locking-mechanism)
- [New electronics](#new-electronics)
  * [Controller board](#controller-board)
  * [I/O module](#io-module)
- [Software](#software)
  * [Configuration](#configuration)
  * [Unlock Key](#unlock-key)
  * [Manual mode](#manual-mode)

# Understanding the locking mechanism
As described above, I've used a cheap safe from a local hardware store. But any cheap safe should do the trick as long as it can be electronically unlocked and easily disassembled. After removing the cover on the inside of the safe, the original electronics board can be removed and the mechanism looks as follows:

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/coil.png" width="50%">

On the left side of the picture the safe is locked. The pin of the coil is pushed upwards by a spring and the mechanism is blocked. As soon as a current is applied to the coil, the pin is pulled down and the safe can be opened. 
The mechanism is also the reason why these safe must be mounted to something solid (e.g. a wall). Otherwise you can hit the safe from the top which pushes down the spring for a few milliseconds in which the safe can be unlocked.

# New electronics
The idea of this project is now to replace the original electronics with a controller which checks the EmlaLock API if it can be opened or not. To be able to connect to the internet I first started using a ESP8266. After a while I ran out of memory while parsing the JSON file delivered by the EmlaLock API. Therefore, I had to switch to the more powerful [ESP32 module](https://www.amazon.de/AZDelivery-ESP32-NodeMCU-Module-Parent/dp/B07Z6CSD9K).
The controller module is extended by a [real-time clock (RTC) module](https://www.amazon.de/AZDelivery-Real-Clock-DS3231-Parent/dp/B07ZQGBH14). As long as the controller has a WiFi connection the time of the RTC module is set using the [network time protocol](https://en.wikipedia.org/wiki/Network_Time_Protocol). In case the WiFi connection is lost the controller is able to get the time from the RTC module even after the power outage. This is important in case the [manual mode](#manual-mode) is used.

The controller and RTC module are both part of the [controller board](#controller-board) which is mounted inside the safe. The [controller board](#controller-board) is extended by the [I/O module](#io-module) mounted to the outside of the safe which contains a [HD44780 display with an I²C converter](https://www.amazon.de/AZDelivery-Bundle-2004-blau-Parent/dp/B07Z6CPTF4) and a [rotary encoder](https://www.amazon.de/WayinTop-Encoder-Drehgeber-Drehwinkelgeber-Druckknopf/dp/B07T3672VK). Since the controller display requires 5V and the controller runs with 3.3V, a [logic level converter](https://www.amazon.de/gp/product/B07LG6RK7L) is added between the controller board and the LCD module.

## Controller board
The controller board contains the ESP32 controller and the RTC as well as some electronics which can open the coil. The controller board is mounted inside the safe to ensure the firmware cannot be modified while the safe is locked. For the power supply of the controller a USB cable without data-pins should be used (either by disconnecting them or using a USB power cable). 

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/EmlaLockSafe_Controller_Board_Schematic.svg" width="66%"><img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/EmlaLockSafe_Controller_Board_Layout.svg" width="33%">

The controller board can be easily soldered to a prototype PCB:

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/ControllerBoard.png" width="66%">

The only SMD part is the MOSFET in its SOT23 housing which can be placed between 3 pads of the PCB.

## I/O module
The I/O module is responsible for displaying the current state and is therefore mounted to the outside of the safe. It consists of:
* HD44780 based LCD module
* HD44780 to I2C converter
* logic level converter
* rotary encoder

As shown in the first picture the module is glued to the safe. This is because the keyhole for the emergency key is below the I/O module.

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/EmlaLockSafe_IO_Module_Schematic.svg" width="66%">

Everything is mounted to a 3D printed case. The STL file can be downloaded from https://github.com/hugo3132/EmlaLockSafe/raw/master/hardware/EmlaLockSafe_IO_Module_Case.7z. The case itself was created using the free version of onshape. If it should be modified you can use the following link https://cad.onshape.com/documents/5e5c868aceb5d7f8533767fd/w/02a796cf0a235e5fc50259cc/e/6ff6b7a15110ccacfdb04374 and modify it to your own needs.

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/DisplayModule_Back.png" width="49%"><img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/DisplayModule_Front.png" width="49%">

For the I/O module no PCB is required. Everything is directly soldered to each other or connected with some cables. 


# Software

doxygen

## Configuration
The software is configured by a file called *config.h*. Since the WiFi password must be entered into this file only a template version (*config_template.h*) of the file is commited. The template must be renamed to *config.h* and the following fields must be updated:
 * **USER_ID**: The user ID used to communicate with the EmlaLock-API. The ID can be found if you log in to EmlaLock, navigate to *Settings > API*.
 * **API_KEY**: The API key used to communicate with the EmlaLock-API. The key can be found if you log in to EmlaLock, navigate to *Settings > API*.
 * **WIFI_SSID**: The name of your WiFi which should be used to connect to the EmlaLock API.
 * **WIFI_PWD**: The password of your WiFi.
 * **TIME_ZONE**: Your local time zone. Please check https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv to select the correct strign for your time zone.

The other configuration parameters must only be modified in case the hardware schematics are modified. It is strongly recommended to use a LCD module with 20x4 characters because some views assume this size of the module and don't use the values defined in *config.h*.

## Unlock key
<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/unlockkey.png" width="50%">


## Manual mode
TBD

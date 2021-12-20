



# EmlaLockSafe
This project is about controlling a cheap digital safe from a local hardware store using the API of emlalock https://www.emlalock.com. 

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/safe.png" width="100%">

# Contents
- [Setup](#setup)
  - [Howto Flash the Software](#howto-flash-the-software)
  - [WiFi Configuration](#wifi-configuration)
  - [Software Configuration](#software-configuration)
- [Hardware Description](#hardware-description)
  - [Understanding the locking mechanism](#understanding-the-locking-mechanism)
  - [New electronics](#new-electronics)
    - [Controller board](#controller-board)
    - [I/O module](#io-module)
- [Software](#software)
  - [Example Views](#Example-Views)
    - [EmlalockUnlockKeyMenu](#EmlalockUnlockKeyMenu)
    - [LockedView](#LockedView)
  - [Manual mode](#manual-mode)

# Setup
## Howto Flash the Software
For the development on arduino the free [Visual Studio Code](https://code.visualstudio.com/) together with the extension [PlatformIO](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) was used. After both are installed and the controller board is connected to the computer, the folder [software](software) must be opened in Visual Studio Code. The first step is to select the PlatformIO extension (1 in the following picture). In the menu of PlatformIO than Build Filesystem Image should be selected to create an image containg everything from the [data](software/data) folder. Next, Upload Filesystem Image must be selected to flash the data folder to the controller board. Finally, Upload should be selected to compile and upload the firmware:

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/FlashControllerBoard.png" width="30%">

## WiFi Configuration
After the firmware is flashed and the filesystem is uploaded the controller should automatically reboot and show the following on the display:

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/WifiSetup.jpg" width="50%">

Since your WiFi credentials are not known yet, the controller creates a new WiFi called "SafeSetup". After connecting to the "SafeSetup" WiFi the WiFi configuration website should open automatically. If not, try to manually navigate to the website http://10.0.0.1. On the website you should select your WiFi and enter the password which is required to connect to the network. After selecting Save Data the controller will reboot and connect to your WiFi. If anything goes wrong, you can always reset everything by flashing the filesystem image (see [Howto Flash the Software](#howto-flash-the-software)).

## Software Configuration
After the controller rebooted and has connected to the WiFi a webserver is started. The address of the webserver is shown in the display:

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/SoftwareConfigurationDisplay.jpg" width="30%">

Since the controller is now part of your normal network it can be reached by any computer, phone, ... connected to your network.

# Hardware Description 
Note: The following might be outdated since it was written for V1.0.0.

## Understanding the locking mechanism
As described above, I've used a [cheap safe from a local hardware store](https://www.bauhaus.info/tresore/moebeleinsatztresor-security-box-mini/p/23519628). But any cheap safe should do the trick as long as it can be electronically unlocked and easily disassembled. Try to look for a safe which is powered by 4 batteries (4 x 1.5V). Then the chances are high that the 5V of the controller board are high enough to pull the coil. The cheaper the safe the easier it might be to drill the holes into it to mount everything. After removing the cover on the inside of the safe, the original electronics board can be removed and the mechanism looks as follows:

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/coil.png" width="50%">

On the left side of the picture the safe is locked. The pin of the coil is pushed upwards by a spring and the mechanism is blocked. As soon as a current is applied to the coil, the pin is pulled down and the safe can be opened. 
The mechanism is also the reason why these safe must be mounted to something solid (e.g. a wall). Otherwise you can hit the safe from the top which pushes down the spring for a few milliseconds in which the safe can be unlocked.

## New electronics
The idea of this project is now to replace the original electronics with a controller which checks the EmlaLock API if it can be opened or not. To be able to connect to the internet I first started using a ESP8266. After a while I ran out of memory while parsing the JSON file delivered by the EmlaLock API. Therefore, I had to switch to the more powerful [ESP32 module](https://www.amazon.de/AZDelivery-ESP32-NodeMCU-Module-Parent/dp/B07Z6CSD9K).
The controller module is extended by a [real-time clock (RTC) module](https://www.amazon.de/AZDelivery-Real-Clock-DS3231-Parent/dp/B07ZQGBH14). As long as the controller has a WiFi connection the time of the RTC module is set using the [network time protocol](https://en.wikipedia.org/wiki/Network_Time_Protocol). In case the WiFi connection is lost the controller is able to get the time from the RTC module even after the power outage. This is important in case the [manual mode](#manual-mode) is used.

The controller and RTC module are both part of the [controller board](#controller-board) which is mounted inside the safe. The [controller board](#controller-board) is extended by the [I/O module](#io-module) mounted to the outside of the safe which contains a [HD44780 display with an I²C converter](https://www.amazon.de/AZDelivery-Bundle-2004-blau-Parent/dp/B07Z6CPTF4) and a [rotary encoder](https://www.amazon.de/WayinTop-Encoder-Drehgeber-Drehwinkelgeber-Druckknopf/dp/B07T3672VK). Since the controller display requires 5V and the controller runs with 3.3V, a [logic level converter](https://www.amazon.de/gp/product/B07LG6RK7L) is added between the controller board and the LCD module.

### Controller board
The controller board contains the ESP32 controller and the RTC as well as some electronics which can open the coil. The controller board is mounted inside the safe to ensure the firmware cannot be modified while the safe is locked. For the power supply of the controller a USB cable without data-pins should be used (either by disconnecting them or using a USB power cable). 

<img src="https://raw.githubusercontent.com/hugo3132/EmlaLockSafe/master/hardware/Schematics/EmlaLockSafe_Controller_Board.svg" width="66%">

The controller board can be easily soldered to a prototype PCB:

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/ControllerBoard.jpg" width="66%">

The only SMD part is the MOSFET in its SOT23 housing which can be placed between 3 pads of the PCB.

### I/O module
The I/O module is responsible for displaying the current state and is therefore mounted to the outside of the safe. It consists of:
* HD44780 based LCD module
* HD44780 to I2C converter
* logic level converter
* rotary encoder

As shown in the first picture the module is glued to the safe. This is because the keyhole for the emergency key is below the I/O module.

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/Schematics/EmlaLockSafe_IO_Module_Schematic.svg" width="66%">

Everything is mounted to a 3D printed case. The STL file can be downloaded from https://github.com/hugo3132/EmlaLockSafe/raw/master/hardware/EmlaLockSafe_IO_Module_Case.7z. The case itself was created using the free version of onshape. If it should be modified you can use the following link https://cad.onshape.com/documents/5e5c868aceb5d7f8533767fd/w/02a796cf0a235e5fc50259cc/e/6ff6b7a15110ccacfdb04374 and modify it to your own needs.

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/DisplayModule_Back.png" width="49%"><img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/DisplayModule_Front.png" width="49%">

For the I/O module no PCB is required. Everything is directly soldered to each other or connected with some cables. 


# Software
The software is documented using doxygen. Please refer to https://hugo3132.github.io/EmlaLockSafe/index.html to open the software documentation.


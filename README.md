# EmlaLockSafe
This project is about controlling a cheap digital safe from a local hardware store using the API of emlalock https://www.emlalock.com. The safe I've got is the following: https://www.bauhaus.info/tresore/moebeleinsatztresor-security-box-mini/p/23519628

# Contents
- [Understanding the locking mechanism](#understanding-the-locking-mechanism)
- [New Electronics](#new-electronics)
  * [How it works](#how-it-works)

# Understanding the locking mechanism
As described above, I've used a cheap safe from a local hardware store. But any cheap safe should do the trick as long as it can be electronically unlocked and easily disassembled. After removing the cover on the inside of the safe, the original electronics board can be removed and the mechanism looks as follows:

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/coil.png" width="50%">

On the left side of the picture the safe is locked. The pin of the coil is pushed upwards by a spring and the mechanism is blocked. As soon as a current is applied to the coil, the pin is pulled down and the safe can be opened. 
The mechanism is also the reason why these safe must be mounted to something solid (e.g. a wall). Otherwise you can hit the safe from the top which pushes down the spring for a few milliseconds in which the safe can be unlocked.

# New electronics
The idea of this project is now to replace the original electronics with a controller which checks the EmlaLock API if it can be opened or not. To be able to connect to the internet I first started using a ESP8266. After a while I ran out of memory while parsing the JSON file delivered by the EmlaLock API. Therefore, I had to switch to the more powerful [ESP32 module](https://www.amazon.de/AZDelivery-ESP32-NodeMCU-Module-Parent/dp/B07Z6CSD9K).
The controller module is extended by a [real-time clock (RTC) module](https://www.amazon.de/AZDelivery-Real-Clock-DS3231-Parent/dp/B07ZQGBH14). As long as the controller has a WiFi connection the time of the RTC module is set using the [network time protocol](https://en.wikipedia.org/wiki/Network_Time_Protocol). In case the WiFi connection is lost the controller is able to get the time from the RTC module even after the power outage. 
TBD...

<img src="https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/unlockkey.png" width="50%">


Changes in API, no wifi, ... 
---> RTC 
---> Key



Display Unit only for I/O


![Controller Board](https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/EmlaLockSafe_IO_Module_Schematic.svg)
![](https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/DisplayModule_Back.png)
![](https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/DisplayModule_Front.png)

![](https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/pictures/ControllerBoard.png)
![Controller Board](https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/EmlaLockSafe_Controller_Board_Schematic.svg)
![Controller Board](https://github.com/hugo3132/EmlaLockSafe/blob/master/hardware/EmlaLockSafe_Controller_Board_Layout.svg)
Controller Board (inside the safe)


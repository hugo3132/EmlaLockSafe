/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../Tools.h"
#include "../configuration/Configuration.h"

#include <MenuView.h>
#include <ViewBase.h>
#include <WiFi.h>
#include <time.h>

namespace views {
/**
 * @brief View displayed while connection to the access point
 */
class WifiConnectingView : public lcd::ViewBase {
public:
  /**
   * @brief Construct the view object
   *
   * @param display pointer to the LCD instance
   */
  WifiConnectingView(LiquidCrystal_PCF8574* display)
    : lcd::ViewBase(display, "WifiConnectingView") {}

public:
  /**
   * @brief Copy constructor - not available
   */
  WifiConnectingView(const WifiConnectingView& other) = delete;

public:
  /**
   * @brief Move constructor - not available
   */
  WifiConnectingView(WifiConnectingView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    display->clear();
    display->setCursor(0, 0);
    display->print("Connecting to wifi:");

    display->setCursor(0, 1);
    display->print(configuration::Configuration::getSingleton().getSsid());

    // Start connecting
    WiFi.disconnect();
    WiFi.setHostname("EmlalockSafe");
    WiFi.mode(WIFI_STA);
    WiFi.begin(configuration::Configuration::getSingleton().getSsid().c_str(),
               configuration::Configuration::getSingleton().getPwd().c_str());
    WiFi.setAutoReconnect(true);

    // Play an animation while the connection is established
    int counter = 0;
    Tools::waitAnimation(display, [&counter]() -> bool {
      counter++;
      if (counter % 25 == 0) {
        // if we've waited for over 2.5s (25 loops), restart wifi....
        // this is required because of an bug in the ESP32 arduino core.
        WiFi.disconnect();
        WiFi.setHostname("EmlalockSafe");
        WiFi.mode(WIFI_STA);
        WiFi.begin(configuration::Configuration::getSingleton().getSsid().c_str(),
                   configuration::Configuration::getSingleton().getPwd().c_str());
        WiFi.setAutoReconnect(true);
      }
      return WiFi.status() != WL_CONNECTED;
    });

    display->clear();
    display->setCursor(0, 0);
    display->print("Setting Timezone:");
    // configure NTP
    configTzTime(configuration::Configuration::getSingleton().getTimezone().c_str(),
                 "pool.ntp.org",
                 "time.nist.gov",
                 "time.google.com");

    // go back to previous view
    activatePreviousView();
  }

public:
  /**
   * @brief called during the loop function
   *
   * @param forceRedraw if true everything should be redrawn
   */
  virtual void tick(const bool& forceRedraw) {
    // not used since activate blocks and immediately returns to the previous
    // view
  }
}; // namespace views
} // namespace views
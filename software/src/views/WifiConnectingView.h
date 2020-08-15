/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../config.h"

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
    : lcd::ViewBase(display) {}

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
    display->print(WIFI_SSID);

    // Start connecting
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    WiFi.setAutoReconnect(true);

    // Play an animation while the connection is established
    int counter = 0;
    waitAnimation([&counter]() -> bool {
      counter++;
      if (counter % 25 == 0) {
        // if we've waited for over 2.5s (25 loops), restart wifi....
        // this is required because of an bug in the ESP32 arduino core.
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PWD);
        WiFi.setAutoReconnect(true);
      }
      return WiFi.status() != WL_CONNECTED;
    });

    display->clear();
    display->setCursor(0, 0);
    display->print("Setting Timezone:");
    // configure NTP
    configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov", "time.google.com");

    // go back to previous view
    activatePreviousView();
  }

protected:
  /**
   * @brief Shows the animation while waiting until the callback returns false
   *
   * @param stillActive callback function checking if the animation should be
   * shown
   */
  void waitAnimation(std::function<bool()> stillActive) {
    while (stillActive()) {
      for (int i = 0; (i < 20) && stillActive(); i++) {
        if (i == 0) {
          display->setCursor(19, 3);
          display->print(' ');
          display->setCursor(i, 3);
          display->print('*');
        }
        else {
          display->setCursor(i - 1, 3);
          display->print(" *");
        }

        delay(100);
      }
    }
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
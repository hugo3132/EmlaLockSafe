/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once
#include "../Tools.h"
#include "../configuration/ConfigurationServer.h"

#include <MenuView.h>

namespace views {
/**
 * @brief preferences menu
 */
class ConfigurationServerView : public lcd::MenuView {
public:
  /**
   * @brief Construct a new menu object
   *
   * @param display pointer to the display instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  ConfigurationServerView(LiquidCrystal_PCF8574* display,
                          RotaryEncoder* encoder,
                          const int& numberOfColumns,
                          const int& numberOfRows)
    : lcd::MenuView(display, "ConfigurationServerView", encoder, "Open in Browser:", numberOfColumns, numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  ConfigurationServerView(const ConfigurationServerView& other) = delete;

public:
  /**
   * @brief Move constructor - not available otherwise we get problems with the callbacks.
   */
  ConfigurationServerView(ConfigurationServerView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    // is this the first time activate is called?
    if (menuItems.empty()) {
      configuration::ConfigurationServer::begin();

      // create menu items
      createMenuItem("http://" + WiFi.localIP().toString(), [this](MenuItem*) {});
      createMenuItem("Change Wifi Settings", [this](MenuItem*) {
        configuration::Configuration::getSingleton().setWifiSettings("", "");
        ESP.restart();
      });
      createMenuItem("Back", [this](MenuItem*) {
        configuration::ConfigurationServer::end();
        activatePreviousView();
      });
    }

    lcd::MenuView::activate();
  }

public:
  /**
   * @brief Overload Tick
   *
   * This is necessary directly after a reboot, localIP returns 0.0.0.0
   */
  virtual void tick(const bool& forceRedraw) {
    static unsigned long nextCheck = millis();

    if (nextCheck < millis()) {
      static String ip = "";
      String tmpIp = WiFi.localIP().toString();
      if (ip != tmpIp) {
        ip = tmpIp;
        menuItems.front().setText("http://" + tmpIp);
      }
      nextCheck = millis() + 100;
    }
    lcd::MenuView::tick(forceRedraw);
  }
};
} // namespace views
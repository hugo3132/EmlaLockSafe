/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once
#include "../Tools.h"
#include "../UsedInterrupts.h"
#include "../configuration/Configuration.h"
#include "../emlalock/EmergencyKey.h"

#include <DialogYesNo.h>
#include <ESP.h>
#include <MenuView.h>
#include <SPIFFS.h>

namespace views {
/**
 * @brief preferences menu
 */
class PreferencesMenu : public lcd::MenuView {
public:
  /**
   * @brief Construct a new menu object
   *
   * @param display pointer to the display instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  PreferencesMenu(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder, const int& numberOfColumns, const int& numberOfRows)
    : lcd::MenuView(display, "PreferencesMenu", encoder, "Preferences", numberOfColumns, numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  PreferencesMenu(const PreferencesMenu& other) = delete;

public:
  /**
   * @brief Move constructor - not available otherwise we get problems with the callbacks.
   */
  PreferencesMenu(PreferencesMenu&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    // is this the first time activate is called?
    if (menuItems.empty()) {

      createMenuItem("Change Wifi Settings", [this](MenuItem*) {
        configuration::Configuration::getSingleton().setWifiSettings("", "");
        ESP.restart();
      });

      createMenuItem("Start Configuration Server", [this](MenuItem*) {
        const auto& timeRestictions = configuration::Configuration::getSingleton().getTimeRestrictions();
        if(!timeRestictions.restrictConfigurationServer || timeRestictions.checkTime()) {
          ViewStore::activateView(ViewStore::ConfigurationServerView);
        }
        else {
          ViewStore::activateView(ViewStore::TimeRestrictedView);
        }
     });

      createMenuItem("Back", [this](MenuItem*) {
        activatePreviousView();
      });
    }

    lcd::MenuView::activate();
  }
};
} // namespace views
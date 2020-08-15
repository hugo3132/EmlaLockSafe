/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "ViewStore.h"

#include <MenuView.h>

namespace views {
/**
 * @brief Emergency menu
 */
class EmergencyMenu : public lcd::MenuView {
public:
  /**
   * @brief Construct a new main menu object
   *
   * @param display pointer to the display instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  EmergencyMenu(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder, const int& numberOfColumns, const int& numberOfRows)
    : lcd::MenuView(display, encoder, "Unlocked Main Menu", numberOfColumns, numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  EmergencyMenu(const EmergencyMenu& other) = delete;

public:
  /**
   * @brief Move constructor - not available otherwise we get problems with the callbacks.
   */
  EmergencyMenu(EmergencyMenu&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    // is this the first time activate is called?
    if (menuItems.empty()) {
      // create menu items
      createMenuItem("Unlock with key", [this](MenuItem*) {
        ViewStore::activateView(ViewStore::EmergencyEnterKeyView);
      });
      createMenuItem("Start Wifi", [this](MenuItem*) {
        ViewStore::activateView(ViewStore::WifiConnectingView);
      });
    }
    lcd::MenuView::activate();
  }
};
} // namespace views
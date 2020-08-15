/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../Tools.h"
#include "ViewStore.h"

#include <MenuView.h>

namespace views {
/**
 * @brief Main menu as long as there is no active session
 */
class UnlockedMainMenu : public lcd::MenuView {
public:
  /**
   * @brief Construct a new main menu object
   *
   * @param display pointer to the display instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  UnlockedMainMenu(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder, const int& numberOfColumns, const int& numberOfRows)
    : lcd::MenuView(display, encoder, "Unlocked Main Menu", numberOfColumns, numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  UnlockedMainMenu(const UnlockedMainMenu& other) = delete;

public:
  /**
   * @brief Move constructor - not available otherwise we get problems with the
   * callbacks.
   */
  UnlockedMainMenu(UnlockedMainMenu&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    // is this the first time activate is called?
    if (menuItems.empty()) {
      // create menu items

      // Open Safe has no click callback since coil is active as long as button
      // is pressed (see tick function)
      createMenuItem("Open Safe"); 


      createMenuItem("Set Custom Timer", [this](MenuItem*) {
        ViewStore::activateView(ViewStore::SetTimerView);
      });
      createMenuItem("Emlalock Unlock Key", [this](MenuItem*) {
        ViewStore::activateView(ViewStore::EmlalockUnlockKeyMenu);
      });
      createMenuItem("Preferences", [this](MenuItem*) {
        ViewStore::activateView(ViewStore::PreferencesMenu);
      });
    }
    lcd::MenuView::activate();
  }

public:
  /**
   * @brief called during the loop function
   *
   * @param forceRedraw if true everything should be redrawn
   */
  virtual void tick(const bool& forceRedraw) {
    // updated the view of the menu
    lcd::MenuView::tick(forceRedraw);

    // update the signal strength symbol
    Tools::tickWifiSymbol(display, forceRedraw);

    // check if the selection is 0 (Open Safe item) and the button is pressed
    if ((selection == 0) && (encoder->getSwitchState())) {
      // activate coil
      digitalWrite(COIL_PIN, 1);
    }
    else {
      // disable coil
      digitalWrite(COIL_PIN, 0);
    }
  }
};
} // namespace views
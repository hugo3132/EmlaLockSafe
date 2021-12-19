/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../Tools.h"
#include "ViewStore.h"

#include <MenuView.h>
#include <Esp.h>

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
    : lcd::MenuView(display, "UnlockedMainMenu", encoder, "Unlocked Main Menu", numberOfColumns, numberOfRows) {}

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

      createMenuItem("Open Safe", [](MenuItem*) {
        ViewStore::activateView(ViewStore::UnlockSafeView);
      });
      createMenuItem("Set Custom Timer", [](MenuItem*) {
        ViewStore::activateView(ViewStore::SetTimerView);
      });
      createMenuItem("Emlalock Unlock Key", [](MenuItem*) {
        ViewStore::activateView(ViewStore::EmlalockUnlockKeyMenu);
      });
      createMenuItem("Preferences", [](MenuItem*) {
        ViewStore::activateView(ViewStore::PreferencesMenu);
      });
      createMenuItem("Hardware Test View", [](MenuItem*) {
        ViewStore::activateView(ViewStore::HardwareTestView);
      });
      createMenuItem("Reboot", [](MenuItem*) {
        ESP.restart();
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
  }
};
} // namespace views
/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once
#include "../configuration/Configuration.h"

#include <DialogYesNo.h>
#include <MenuView.h>

namespace views {
/**
 * @brief Menu for showing / changing the unlock key
 */
class EmlalockUnlockKeyMenu : public lcd::MenuView {
public:
  /**
   * @brief Construct a new menu object
   *
   * @param display pointer to the display instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  EmlalockUnlockKeyMenu(LiquidCrystal_PCF8574* display,
                        RotaryEncoder* encoder,
                        const int& numberOfColumns,
                        const int& numberOfRows)
    : lcd::MenuView(display, "EmlalockUnlockKeyMenu", encoder, "Emlalock Unlock Key", numberOfColumns, numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  EmlalockUnlockKeyMenu(const EmlalockUnlockKeyMenu& other) = delete;

public:
  /**
   * @brief Move constructor - not available otherwise we get problems with the callbacks.
   */
  EmlalockUnlockKeyMenu(EmlalockUnlockKeyMenu&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    // is this the first time activate is called?
    if (menuItems.empty()) {
      // Menu item just showing the current key
      String keyItemText = "Current Key: " + configuration::Configuration::getSingleton().getEmergencyKey();
      createMenuItem(keyItemText, [this](MenuItem*) {});

      // Menu item which allows to generate a new key
      createMenuItem("Generate new key", [this](MenuItem*) {
        if (lcd::DialogYesNo(display,
                             encoder,
                             "Are you sure?\nThe old key wont't\nbe valid anymore.",
                             numberOfColumns,
                             numberOfRows)
              .showModal(false)) {
          // update the menu item
          String keyItemText = "Current key: " + configuration::Configuration::getSingleton().generateNewEmergencyKey();
          menuItems.front().setText(keyItemText);
        }
      });

      // Item for going back to the previous menu.
      createMenuItem("Back", [this](MenuItem*) {
        activatePreviousView();
      });
    }

    lcd::MenuView::activate();
  }
};
} // namespace views
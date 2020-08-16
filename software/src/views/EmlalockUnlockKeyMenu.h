/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once
#include "../emlalock/EmergencyKey.h"

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
    char key[7];

    // get the current key from the file system
    emlalock::EmergencyKey::getCurrentKey(key);
    String keyItemText = "Current Key: " + String(key);

    // is this the first time activate is called?
    if (menuItems.empty()) {
      // create menu items
      createMenuItem(keyItemText, [this](MenuItem*) {});

      createMenuItem("Generate new key", [this](MenuItem*) {
        if (lcd::DialogYesNo(display,
                             encoder,
                             "Are you sure?\nThe old key wont't\nbe valid anymore.",
                             numberOfColumns,
                             numberOfRows)
              .showModal(false)) {
          char key[7];
          // generate and save new key
          emlalock::EmergencyKey::generateNewKey(key);
        }
      });

      createMenuItem("Back", [this](MenuItem*) {
        activatePreviousView();
      });
    }
    else {
      menuItems.front().setText(keyItemText);
    }

    lcd::MenuView::activate();
  }
};
} // namespace views
/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once
#include "../Tools.h"
#include "../emlalock/EmergencyKey.h"

#include <DialogYesNo.h>
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
    : lcd::MenuView(display, encoder, "Preferences", numberOfColumns, numberOfRows) {}

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
      // create menu items
      createMenuItem("Format Flash", [this](MenuItem*) {
        onFormatFlash();
      });

      createMenuItem("Back", [this](MenuItem*) {
        activatePreviousView();
      });
    }

    lcd::MenuView::activate();
  }

protected:
  /**
   * @brief Callback if the flash should be formatted
   */
  void onFormatFlash() {
    if (lcd::DialogYesNo(display, encoder, "Are you sure to\nformat the\nfilesystem?", numberOfColumns, numberOfRows)
          .showModal(false)) {
      display->clear();
      display->setCursor(0, 0);
      display->print("Formatting Flash...");
      display->setCursor(0, 1);
      display->print("This will take a");
      display->setCursor(0, 2);
      display->print("while! Input must be");
      display->setCursor(0, 3);
      display->print("disabled.");
      // Backup the unlock key to memory
      char key[7];
      emlalock::EmergencyKey::getCurrentKey(key);

      // detach interrputs which would crash the ESP while accessing the
      // file-system
      Tools::detachEncoderInterrupts();
      SPIFFS.format();
      Tools::attachEncoderInterrupts();

      // Save the key back to memory
      emlalock::EmergencyKey::setKey(key);

      // reset display
      lcd::MenuView::activate();
    }
  }
};
} // namespace views
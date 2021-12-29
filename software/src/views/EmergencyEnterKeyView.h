/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../LockState.h"
#include "../configuration/Configuration.h"
#include "ViewStore.h"

#include <RotaryEncoder.h>
#include <ViewBase.h>

namespace views {
/**
 * @brief View which allows to enter a key to unlock without WiFi
 */
class EmergencyEnterKeyView : public lcd::ViewBase {
protected:
  /**
   * @brief pointer to the encoder instance
   */
  RotaryEncoder* encoder;

protected:
  /**
   * @brief Cache of the entered key
   */
  char enteredKey[7];

protected:
  /**
   * @brief Position of the key cache which is currently edited
   */
  int editIndex;

public:
  /**
   * @brief Construct the view object
   *
   * @param display pointer to the LCD instance
   */
  EmergencyEnterKeyView(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder)
    : lcd::ViewBase(display, "EmergencyEnterKeyView")
    , encoder(encoder) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  EmergencyEnterKeyView(const EmergencyEnterKeyView& other) = delete;

public:
  /**
   * @brief Move constructor
   */
  EmergencyEnterKeyView(EmergencyEnterKeyView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    strcpy(enteredKey, "AAAAAA");
    editIndex = 0;

    display->clear();
    display->setCursor(0, 0);
    display->print("Enter emergency key:");
    tick(true);
  }

public:
  /**
   * @brief called during the loop function
   *
   * @param forceRedraw if true everything should be redrawn
   */
  virtual void tick(const bool& forceRedraw) {
    auto direction = encoder->getDirection();
    bool redraw = forceRedraw;

    if (direction == RotaryEncoder::Direction::CLOCKWISE) {
      if (enteredKey[editIndex] == 'Z') {
        enteredKey[editIndex] = '0';
      }
      else if (enteredKey[editIndex] == '9') {
        enteredKey[editIndex] = 'A';
      }
      else {
        enteredKey[editIndex]++;
      }
      redraw = true;
    }
    else if (direction == RotaryEncoder::Direction::COUNTERCLOCKWISE) {
      if (enteredKey[editIndex] == 'A') {
        enteredKey[editIndex] = '9';
      }
      else if (enteredKey[editIndex] == '0') {
        enteredKey[editIndex] = 'Z';
      }
      else {
        enteredKey[editIndex]--;
      }
      redraw = true;
    }

    if (encoder->getNewClick()) {
      editIndex++;
      if (editIndex == 6) {
        editIndex = 0;
        display->setCursor(0, 1);

        if (strcmp(enteredKey, configuration::Configuration::getSingleton().getEmergencyKey().c_str()) == 0) {
          const auto& timeRestictions = configuration::Configuration::getSingleton().getTimeRestrictions();
          if (!timeRestictions.restrictEmergencyKeyTimes || timeRestictions.checkTime()) {
            // Correct key was entered...
            LockState::setEndDate(0); // set to unlock
            LockState::setLastUpdateTime(time(NULL) + 10);
            ViewStore::activateView(ViewStore::UnlockSafeView);
            return;
          }
          else {
            ViewStore::activateView(ViewStore::TimeRestrictedView);
            return;
          }
        }
        else {
          display->print("Invalid Key");
        }
      }
      redraw = true;
    }

    if (redraw) {
      for (int i = 0; i < 6; i++) {
        display->setCursor(2 * i + 3, 3);
        display->print(' ');
        display->print(enteredKey[i]);
      }
      display->print(' ');

      display->setCursor(editIndex * 2 + 3, 3);
      display->print('>');
      display->setCursor(editIndex * 2 + 5, 3);
      display->print('<');
    }
  }
};
} // namespace views
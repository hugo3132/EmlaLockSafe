/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../LockState.h"
#include "../Tools.h"
#include "ViewStore.h"

#include <DialogOK.h>
#include <MenuView.h>
#include <RotaryEncoder.h>
#include <ViewBase.h>
#include <time.h>

namespace views {
/**
 * @brief View used to unlock the safe
 */
class UnlockSafeView : public lcd::ViewBase {
protected:
  /**
   * @brief pointer to the encoder instance
   */
  RotaryEncoder* encoder;

protected:
  /**
   * @brief Number of display-columns
   */
  const int numberOfColumns;

protected:
  /**
   * @brief Number of display-rows
   */
  const int numberOfRows;

protected:
  /**
   * @brief Time when this view was activated
   */
  time_t activationTime;

public:
  /**
   * @brief Construct the vie
   *
   * @param display pointer to the LCD instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  UnlockSafeView(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder, const int& numberOfColumns, const int& numberOfRows)
    : lcd::ViewBase(display, "UnlockSafeView")
    , encoder(encoder)
    , numberOfColumns(numberOfColumns)
    , numberOfRows(numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  UnlockSafeView(const UnlockSafeView& other) = delete;

public:
  /**
   * @brief Move constructor - not available
   */
  UnlockSafeView(UnlockSafeView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    display->clear();
    display->setCursor(0, 0);
    display->print("Verifying Emlalock");

    activationTime = time(NULL);
    emlalock::EmlaLockApi::getSingleton().triggerRefresh();

    // since tick is blocking we should never call it directly from here...
  }

public:
  /**
   * @brief called during the loop function
   *
   * @param forceRedraw if true everything should be redrawn
   */
  virtual void tick(const bool& forceRedraw) {
    digitalWrite(COIL_PIN, 0);

    // Show the wait animation
    Tools::waitAnimation(display, [this]() -> bool {
      return (LockState::getLastUpdateTime() < activationTime) && (time(NULL) - activationTime < 15);
    });

    // Did we run in a 15s timeout?
    if (LockState::getLastUpdateTime() < activationTime) {
      lcd::DialogOk(display, encoder, "The Emlalock API\ndid not answer\nwithin 15s.", numberOfColumns, numberOfRows).showModal();
      activatePreviousView();
      return;
    }

    // Should the safe be locked
    if (LockState::getEndDate() > time(NULL)) {
      digitalWrite(COIL_PIN, 0);
      return;
    }

    // Activate the coil if necessary
    static unsigned long coilActivationTime = 0;
    if (!digitalRead(COIL_PIN)) {
      display->clear();
      display->setCursor(0, 0);
      display->print("Safe unlocked");
      coilActivationTime = millis();
      digitalWrite(COIL_PIN, 1);
    }

    // show a animation showing how many time is lest until the coil is released
    // again
    display->setCursor(0, 1);
    for (int i = 0; i < (millis() - coilActivationTime) / 500; i++) {
      display->write(0xFF);
    }

    // Check if the coil should be released after 10 seconds
    if (millis() - coilActivationTime > 10000) {
      digitalWrite(COIL_PIN, 0);
      activatePreviousView();
    }
  }
};
} // namespace views
/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../LockState.h"
#include "ViewStore.h"

#include <RotaryEncoder.h>
#include <ViewBase.h>

namespace views {
/**
 * @brief View used when the safe is unlocked for a hygiene opening
 */
class HygieneOpeningView : public lcd::ViewBase {
protected:
  /**
   * @brief pointer to the encoder instance
   */
  RotaryEncoder* encoder;

public:
  /**
   * @brief Construct the locked view
   *
   * @param display pointer to the LCD instance
   */
  HygieneOpeningView(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder)
    : lcd::ViewBase(display, "HygieneOpeningView")
    , encoder(encoder) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  HygieneOpeningView(const HygieneOpeningView& other) = delete;

public:
  /**
   * @brief Move constructor
   */
  HygieneOpeningView(HygieneOpeningView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
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
    auto click = encoder->getNewClick();
    char buf[21];

    if (click || (direction != RotaryEncoder::Direction::NOROTATION)) {
      if (!getBacklightTimeoutManager().delayTimeout()) {
        return;
      }
    }
    getBacklightTimeoutManager().tick(display);

    // click unlocks the coil
    if (click) {
      ViewStore::activateView(ViewStore::UnlockSafeView);
    }

    // Stuff which never changes
    if (forceRedraw) {
      display->clear();
      display->setCursor(0, 0);
      display->print("Hygiene Opening");

      display->setCursor(0, 1);
      display->print("Time Left:  **:**:**");

      display->setCursor(0, 3);
      display->print("Click to open safe!");
    }

    // should the time left be displayed? If yes, how
    static time_t lastDisplayedTime = 0;
    time_t timeLeft = LockState::getCleaningEndDate() - time(NULL);

    // redraw required?
    if (forceRedraw || (lastDisplayedTime != timeLeft)) {
      lastDisplayedTime = timeLeft;

      int sec = timeLeft % 60;
      timeLeft = timeLeft / 60;
      int min = timeLeft % 60;
      timeLeft = timeLeft / 60;
      int hour = timeLeft;

      sprintf(buf, "Time Left:  %02d:%02d:%02d", hour, min, sec);
      display->setCursor(0, 1);
      display->print(buf);
    }
  }
};
} // namespace views
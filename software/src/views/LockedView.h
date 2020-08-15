/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../config.h"
#include "../Tools.h"

#include <RotaryEncoder.h>
#include <ViewBase.h>

namespace views {
/**
 * @brief View used when the safe is locked.
 */
class LockedView : public lcd::ViewBase {
protected:
  /**
   * @brief pointer to the encoder instance
   */
  RotaryEncoder* encoder;

protected:
public:
  /**
   * @brief Construct the locked view
   *
   * @param display pointer to the LCD instance
   */
  LockedView(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder)
    : lcd::ViewBase(display)
    , encoder(encoder) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  LockedView(const LockedView& other) = delete;

public:
  /**
   * @brief Move constructor
   */
  LockedView(LockedView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    display->clear();
    display->setCursor(0, 0);
    display->print("Locked");

    initializeSpecialCharacters();
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
    if (click || (direction != RotaryEncoder::Direction::NOROTATION)) {
      if (!getBacklightTimeoutManager().delayTimeout()) {
        return;
      }
    }
    getBacklightTimeoutManager().tick(display);

    digitalWrite(COIL_PIN, 0);

    Tools::tickWifiSymbol(display, forceRedraw);
  }
};
} // namespace views
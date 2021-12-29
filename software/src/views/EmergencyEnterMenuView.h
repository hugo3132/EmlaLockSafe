/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include <ViewBase.h>

namespace views {
/**
 * @brief A view which is displayed for a view seconds when the button is
 * pressed during boot.
 */
class EmergencyEnterMenuView : public lcd::ViewBase {
public:
  /**
   * @brief Construct a vie object
   *
   * @param display pointer to the LCD instance
   */
  EmergencyEnterMenuView(LiquidCrystal_PCF8574* display)
    : lcd::ViewBase(display, "EmergencyEnterMenuView") {}

public:
  /**
   * @brief Copy constructor - not available
   */
  EmergencyEnterMenuView(const EmergencyEnterMenuView& other) = delete;

public:
  /**
   * @brief Move constructor - not available
   */
  EmergencyEnterMenuView(EmergencyEnterMenuView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    display->clear();
    display->setCursor(0, 0);
    display->print("Keep button pressed");
    display->setCursor(0, 1);
    display->print("to enter emergency");
    display->setCursor(0, 2);
    display->print("menu.");
    display->setCursor(0, 3);
  }

public:
  /**
   * @brief called during the loop function
   *
   * @param forceRedraw if true everything should be redrawn
   */
  virtual void tick(const bool& forceRedraw) {
    display->print('*');
  }
};
} // namespace views
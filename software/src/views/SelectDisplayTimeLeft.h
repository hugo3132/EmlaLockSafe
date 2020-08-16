/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once
#include "../LockState.h"
#include "ViewStore.h"

#include <MenuView.h>
#include <time.h>

namespace views {
/**
 * @brief selection menu if the time passed should be displayed or not
 */
class SelectDisplayTimeLeft : public lcd::MenuView {
public:
  /**
   * @brief Construct a new menu object
   *
   * @param display pointer to the display instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  SelectDisplayTimeLeft(LiquidCrystal_PCF8574* display,
                        RotaryEncoder* encoder,
                        const int& numberOfColumns,
                        const int& numberOfRows)
    : lcd::MenuView(display, "SelectDisplayTimeLeft", encoder, "Display time left?", numberOfColumns, numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  SelectDisplayTimeLeft(const SelectDisplayTimeLeft& other) = delete;

public:
  /**
   * @brief Move constructor - not available otherwise we get problems with the callbacks.
   */
  SelectDisplayTimeLeft(SelectDisplayTimeLeft&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    // is this the first time activate is called?
    if (menuItems.empty()) {
      // create menu items
      createMenuItem("Yes", [this](MenuItem*) {
        LockState::setDisplayTimeLeft(LockState::DisplayTimeLeft::yes);
        LockState::setStartDate(time(NULL));
        LockState::setTemperatureString("");
        LockState::setEndDate(LockState::getCachedEndDate());
      });

      createMenuItem("No", [this](MenuItem*) {
        LockState::setDisplayTimeLeft(LockState::DisplayTimeLeft::no);
        LockState::setStartDate(time(NULL));
        LockState::setTemperatureString("");
        LockState::setEndDate(LockState::getCachedEndDate());
      });

      createMenuItem("Temperature", [this](MenuItem*) {
        LockState::setDisplayTimeLeft(LockState::DisplayTimeLeft::temperature);
        LockState::setStartDate(time(NULL));
        LockState::setTemperatureString(""); // Automatically generated!
        LockState::setEndDate(LockState::getCachedEndDate());
      });

      createMenuItem("Cancel", [this](MenuItem*) {
        views::ViewStore::activateView(views::ViewStore::UnlockedMainMenu);
      });
    }

    lcd::MenuView::activate();
  }
};
} // namespace views
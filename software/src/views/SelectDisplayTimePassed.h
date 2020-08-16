/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once
#include "../LockState.h"
#include "ViewStore.h"

#include <MenuView.h>

namespace views {
/**
 * @brief selection menu if the time passed should be displayed or not
 */
class SelectDisplayTimePassed : public lcd::MenuView {
public:
  /**
   * @brief Construct a new menu object
   *
   * @param display pointer to the display instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  SelectDisplayTimePassed(LiquidCrystal_PCF8574* display,
                          RotaryEncoder* encoder,
                          const int& numberOfColumns,
                          const int& numberOfRows)
    : lcd::MenuView(display, "SelectDisplayTimePassed", encoder, "Display time passed?", numberOfColumns, numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  SelectDisplayTimePassed(const SelectDisplayTimePassed& other) = delete;

public:
  /**
   * @brief Move constructor - not available otherwise we get problems with the callbacks.
   */
  SelectDisplayTimePassed(SelectDisplayTimePassed&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    // is this the first time activate is called?
    if (menuItems.empty()) {
      // create menu items
      createMenuItem("Yes", [this](MenuItem*) {
        LockState::setDisplayTimePassed(LockState::DisplayTimePassed::yes);
        views::ViewStore::activateView(views::ViewStore::SelectDisplayTimeLeft);
      });

      createMenuItem("No", [this](MenuItem*) {
        LockState::setDisplayTimePassed(LockState::DisplayTimePassed::no);
        views::ViewStore::activateView(views::ViewStore::SelectDisplayTimeLeft);
      });

      createMenuItem("Cancel", [this](MenuItem*) {
        views::ViewStore::activateView(views::ViewStore::UnlockedMainMenu);
      });
    }

    lcd::MenuView::activate();
  }
};
} // namespace views
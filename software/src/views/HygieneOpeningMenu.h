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
 * @brief View used when the safe is unlocked for a hygiene opening
 */
class HygieneOpeningMenu : public lcd::MenuView {
public:
  /**
   * @brief Construct a new hygiene opening menu object
   *
   * @param display pointer to the display instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  HygieneOpeningMenu(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder, const int& numberOfColumns, const int& numberOfRows)
    : lcd::MenuView(display, "HygieneOpeningMenu", encoder, "Hygiene Opening", numberOfColumns, numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  HygieneOpeningMenu(const HygieneOpeningMenu& other) = delete;

public:
  /**
   * @brief Move constructor
   */
  HygieneOpeningMenu(HygieneOpeningMenu&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    // is this the first time activate is called?
    if (menuItems.empty()) {
      // create menu items
      createMenuItem("Open Safe", [](MenuItem*) {
        ViewStore::activateView(ViewStore::UnlockSafeView);
      });
      createMenuItem("Emlalock Unlock Key", [](MenuItem*) {
        ViewStore::activateView(ViewStore::EmlalockUnlockKeyMenu);
      });
      createMenuItem("Time Left: 00:00:00", [](MenuItem*) {});
    }
    lcd::MenuView::activate();
  }

public:
  /**
   * @brief called during the loop function
   *
   * @param forceRedraw if true everything should be redrawn
   */
  virtual void tick(const bool& forceRedraw) {
    // updated the view of the menu
    lcd::MenuView::tick(forceRedraw);

    auto direction = encoder->getDirection();
    auto click = encoder->getNewClick();
    char buf[21];

    if (click || (direction != RotaryEncoder::Direction::NOROTATION)) {
      if (!getBacklightTimeoutManager().delayTimeout()) {
        return;
      }
    }
    getBacklightTimeoutManager().tick(display);

    // should the time left be displayed? If yes, how
    static time_t lastDisplayedTime = 0;
    time_t timeLeft = 0;
    if(LockState::getCleaningEndDate() > time(NULL)) {
      timeLeft = LockState::getCleaningEndDate() - time(NULL);
    }

    // redraw required?
    if (forceRedraw || (lastDisplayedTime != timeLeft)) {
      lastDisplayedTime = timeLeft;

      int sec = timeLeft % 60;
      timeLeft = timeLeft / 60;
      int min = timeLeft % 60;
      timeLeft = timeLeft / 60;
      int hour = timeLeft;

      sprintf(buf, "Time Left: %02d:%02d:%02d", hour, min, sec);

      menuItems.back().setText(buf);
    }
  }
};
} // namespace views
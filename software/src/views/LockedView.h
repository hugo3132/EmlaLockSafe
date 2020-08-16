/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../Tools.h"
#include "../config.h"

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
  /**
   * @brief temperature strings if they are not provided by the emlalock API
   */
  String temperatureStrings[6];

public:
  /**
   * @brief Construct the locked view
   *
   * @param display pointer to the LCD instance
   */
  LockedView(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder)
    : lcd::ViewBase(display)
    , encoder(encoder) {
    temperatureStrings[0] = "very cold";
    temperatureStrings[1] = "cold";
    temperatureStrings[2] = "warm";
    temperatureStrings[3] = "very warm";
    temperatureStrings[4] = "hot";
    temperatureStrings[5] = "very hot";
  }

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
    initializeSpecialCharacters();

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
    tm tmBuf;

    if (click || (direction != RotaryEncoder::Direction::NOROTATION)) {
      if (!getBacklightTimeoutManager().delayTimeout()) {
        return;
      }
    }
    getBacklightTimeoutManager().tick(display);

    // just to be sure...
    digitalWrite(COIL_PIN, 0);

    // Stuff which never changes
    if (forceRedraw) {
      display->clear();
      display->setCursor(0, 0);
      display->print("Safe Locked");

      display->setCursor(0, 1);
      display->print("Passed: ***:**:**:**");
      display->setCursor(0, 2);
      display->print("Left:   ***:**:**:**");

      if (LockState::getDisplayTimeLeft() == LockState::DisplayTimeLeft::yes) {
        strftime(buf, 21, " %d.%m.%Y %T", localtime_r(&LockState::getEndDate(), &tmBuf));
        display->setCursor(0, 3);
        display->print(buf);
      }
    }

    // current time
    static time_t lastDisplayedCurrentTime = 0;
    time_t currentTime = time(NULL);
    if (forceRedraw || (lastDisplayedCurrentTime/60 != currentTime/60)) {
      lastDisplayedCurrentTime = currentTime;
      strftime(buf, 6, "%R", localtime_r(&currentTime, &tmBuf));
      display->setCursor(13, 0);
      display->print(buf);
    }

    // draw wifi signal strength
    Tools::tickWifiSymbol(display, forceRedraw);

    // should the passed time be displayed?
    if (LockState::getDisplayTimePassed() == LockState::DisplayTimePassed::yes) {
      static time_t lastDisplayedTime = 0;
      time_t timePassed = time(NULL) - LockState::getStartDate();

      // redraw required
      if (forceRedraw || (lastDisplayedTime != timePassed)) {
        lastDisplayedTime = timePassed;

        int sec = timePassed % 60;
        timePassed = timePassed / 60;
        int min = timePassed % 60;
        timePassed = timePassed / 60;
        int hour = timePassed % 24;
        int day = std::min((int)timePassed / 24, 999);

        sprintf(buf, "Passed: %03d:%02d:%02d:%02d", day, hour, min, sec);
        display->setCursor(0, 1);
        display->print(buf);
      }
    }

    // should the time left be displayed? If yes, how
    switch (LockState::getDisplayTimeLeft()) {
    case LockState::DisplayTimeLeft::yes: {
      static time_t lastDisplayedTime = 0;
      time_t timeLeft = LockState::getEndDate() - time(NULL);

      // redraw required?
      if (forceRedraw || (lastDisplayedTime != timeLeft)) {
        lastDisplayedTime = timeLeft;

        int sec = timeLeft % 60;
        timeLeft = timeLeft / 60;
        int min = timeLeft % 60;
        timeLeft = timeLeft / 60;
        int hour = timeLeft % 24;
        int day = std::min((int)timeLeft / 24, 999);

        sprintf(buf, "Left:   %03d:%02d:%02d:%02d", day, hour, min, sec);
        display->setCursor(0, 2);
        display->print(buf);
      }

      break;
    }
    case LockState::DisplayTimeLeft::temperature: {
      static String lastString = "";
      String s = LockState::getTemperatureString();

      // The string does not come from the Emlalock API. Just generate it
      if (s.isEmpty()) {
        int idx = std::min(
          (int)(((time(NULL) - LockState::getStartDate()) * 6) / (LockState::getEndDate() - LockState::getStartDate())),
          5);

        s = temperatureStrings[idx];
      }

      // redraw required?
      if (forceRedraw || (s != lastString)) {
        display->setCursor(0, 2);
        display->print("Left:               ");
        display->setCursor(6, 2);
        display->print(s);
        lastString = s;
      }
      break;
    }
    default:
      break;
    }
  }
};
} // namespace views
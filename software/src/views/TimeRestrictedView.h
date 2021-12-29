/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../configuration/Configuration.h"

#include <ViewBase.h>

namespace views {
/**
 * @brief View used when the safe should be opened but the time is not within the allowed range
 */
class TimeRestrictedView : public lcd::ViewBase {
protected:
  /**
   * @brief Time when the view was activated
   */
  long activationMillis;

public:
  /**
   * @brief Construct the view
   *
   * @param display pointer to the LCD instance
   */
  TimeRestrictedView(LiquidCrystal_PCF8574* display)
    : lcd::ViewBase(display, "TimeRestrictedView") {}

public:
  /**
   * @brief Copy constructor - not available
   */
  TimeRestrictedView(const TimeRestrictedView& other) = delete;

public:
  /**
   * @brief Move constructor
   */
  TimeRestrictedView(TimeRestrictedView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    getBacklightTimeoutManager().delayTimeout();
    activationMillis = millis();
    tick(true);
  }

public:
  /**
   * @brief called during the loop function
   *
   * @param forceRedraw if true everything should be redrawn
   */
  virtual void tick(const bool& forceRedraw) {
    getBacklightTimeoutManager().delayTimeout();
    if (activationMillis + 10000 < millis()) {
      activatePreviousView();
    }

    // Stuff which never changes
    if (forceRedraw) {
      const auto& restrictions = configuration::Configuration::getSingleton().getTimeRestrictions();

      display->clear();
      display->setCursor(0, 0);
      display->print("Open Time Restricted");

      display->setCursor(0, 1);
      int h = restrictions.startTime / 3600;
      int m = (restrictions.startTime / 60) % 60;
      display->printf("Start Time:    %02d:%02d", h, m);
      display->setCursor(0, 2);
      h = restrictions.endTime / 3600;
      m = (restrictions.startTime / 60) % 60;
      display->printf("End Time:      %02d:%02d", h, m);

      display->setCursor(0, 3);
      display->print("Current Time:  00:00");
    }

    // current time
    static time_t lastDisplayedCurrentTime = 0;
    time_t currentTime = time(NULL);
    if (forceRedraw || (lastDisplayedCurrentTime / 60 != currentTime / 60)) {
      char buf[21];
      tm tmBuf;
      lastDisplayedCurrentTime = currentTime;
      strftime(buf, 6, "%R", localtime_r(&currentTime, &tmBuf));
      display->setCursor(15, 3);
      display->print(buf);
    }
  }
};
} // namespace views
/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../Tools.h"

#include <RotaryEncoder.h>
#include <ViewBase.h>
#include <ds3231.h>

namespace views {
/**
 * @brief Main view testing all connected hardware
 */
class HardwareTestView : public lcd::ViewBase {
protected:
  /**
   * @brief pointer to the encoder instance
   */
  RotaryEncoder* encoder;

public:
  /**
   * @brief Construct a vie object
   *
   * @param display pointer to the LCD instance
   */
  HardwareTestView(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder)
    : lcd::ViewBase(display)
    , encoder(encoder) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  HardwareTestView(const HardwareTestView& other) = delete;

public:
  /**
   * @brief Move constructor
   */
  HardwareTestView(HardwareTestView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    display->clear();
    display->setCursor(0, 0);
    display->print("Encoder Test");
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

    // Display the state of the encoder
    if (click) {
      display->setCursor(0, 1);
      display->print("Clicked");
      delay(500);
      display->setCursor(0, 1);
      display->print("                 ");
    }

    if (direction == RotaryEncoder::Direction::CLOCKWISE) {
      display->setCursor(0, 1);
      display->print("CLOCKWISE");
      delay(500);
      display->setCursor(0, 1);
      display->print("                 ");
    }

    if (direction == RotaryEncoder::Direction::COUNTERCLOCKWISE) {
      display->setCursor(0, 1);
      display->print("COUNTERCLOCKWISE");
      delay(500);
      display->setCursor(0, 1);
      display->print("                 ");
    }

    // Get current time
    struct ts rtcTime;
    DS3231_get(&rtcTime);

    // convert RTC structure to tm structure
    tm timeBuf;
    char buf[21];
    timeBuf.tm_year = rtcTime.year - 1900;
    timeBuf.tm_mon = rtcTime.mon - 1;
    timeBuf.tm_mday = rtcTime.mday;
    timeBuf.tm_hour = rtcTime.hour;
    timeBuf.tm_min = rtcTime.min;
    timeBuf.tm_sec = rtcTime.sec;
    strftime(buf, 21, "R %d.%m.%y %T", &timeBuf);
    display->setCursor(0, 2);
    display->print(buf);

    static time_t lastTime = 0;
    time_t now = time(NULL);
    if (now != lastTime) {
      strftime(buf, 21, "S %d.%m.%y %T", localtime_r(&now, &timeBuf));
      display->setCursor(0, 3);
      display->print(buf);
      lastTime = now;
    }

    Tools::tickWifiSymbol(display, forceRedraw);
  }
};
} // namespace views
/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "LockState.h"
#include "ViewStore.h"

#include <DialogYesNoBack.h>
#include <MenuView.h>
#include <RotaryEncoder.h>
#include <ViewBase.h>

namespace views {
/**
 * @brief View used to set a manual timer without EmlaLock in the background
 */
class SetTimerView : public lcd::ViewBase {
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
   * @brief current selection of the number of days
   */
  uint8_t numberOfDays;

protected:
  /**
   * @brief current selection of the number of hours
   */
  uint8_t numberOfHours;

protected:
  /**
   * @brief current selection of the number of minutes
   */
  uint8_t numberOfMinutes;

protected:
  /**
   * @brief selection what field can be changed:
   *
   * 0: number of days
   * 1: number of hours
   * 2: number of minutes
   */
  uint8_t editIndex;

public:
  /**
   * @brief Construct the new set timer view
   *
   * @param display pointer to the LCD instance
   * @param encoder pointer to the encoder instance
   * @param numberOfColumns number of display-columns
   * @param numberOfRows number of display-rows
   */
  SetTimerView(LiquidCrystal_PCF8574* display, RotaryEncoder* encoder, const int& numberOfColumns, const int& numberOfRows)
    : lcd::ViewBase(display)
    , encoder(encoder)
    , numberOfColumns(numberOfColumns)
    , numberOfRows(numberOfRows) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  SetTimerView(const SetTimerView& other) = delete;

public:
  /**
   * @brief Move constructor - not available
   */
  SetTimerView(SetTimerView&& other) noexcept = delete;

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    display->clear();

    // reset the values
    numberOfDays = 0;
    numberOfHours = 0;
    numberOfMinutes = 0;
    editIndex = 0;

    // redraw everything
    tick(true);
  }

public:
  /**
   * @brief called during the loop function
   *
   * @param forceRedraw if true everything should be redrawn
   */
  virtual void tick(const bool& forceRedraw) {
    bool redraw = forceRedraw;

    // handle the encoder
    handleEncoderRotation(redraw);
    if (!handleEncoderClick(redraw)) {
      return;
    }

    // redraw the timer settings
    if (redraw) {
      display->setCursor(0, 0);
      display->print("Set Lock Timer:");

      display->setCursor(0, 1);
      display->printf("%s%d%cdays  ", (editIndex == 0 ? ">" : ""), numberOfDays, (editIndex == 0 ? '<' : ' '));
      display->setCursor(0, 2);
      char buf[25];
      sprintf(buf,
              "%s%d%chours %s%d%cminutes  ",
              (editIndex == 1 ? ">" : ""),
              numberOfHours,
              (editIndex == 1 ? '<' : ' '),
              (editIndex == 2 ? ">" : ""),
              numberOfMinutes,
              (editIndex == 2 ? '<' : ' '));
      buf[20] = '\0';
      display->print(buf);
    }

    // redraw end-time
    time_t setTime = time(NULL) + ((((time_t)numberOfDays) * 24 + ((time_t)numberOfHours)) * 60 + ((time_t)numberOfMinutes)) * 60;
    static time_t lastDisplayedTime = 0;
    if (redraw || (setTime != lastDisplayedTime)) {
      tm tmBuf;
      char buf[21];
      strftime(buf, 20, "(%d.%m.%Y %R)", localtime_r(&setTime, &tmBuf));
      display->setCursor(0, 3);
      display->print(buf);

      lastDisplayedTime = setTime;
    }
  }

protected:
  /**
   * @brief handles if the encoder was rotated
   *
   * @param[in, out] redraw true if the content of the display must be redraw
   */
  void handleEncoderRotation(bool& redraw) {
    // read the state of the encoder
    auto direction = encoder->getDirection();

    // increase the selected value?
    if (direction == RotaryEncoder::Direction::CLOCKWISE) {
      switch (editIndex) {
      case 0:
        numberOfDays++;
        break;
      case 1:
        numberOfHours++;
        if (numberOfHours == 24) {
          numberOfHours = 0;
        }
        break;
      case 2:
        numberOfMinutes++;
        if (numberOfMinutes == 60) {
          numberOfMinutes = 0;
        }
        break;
      }
      redraw = true; // redraw required
    }

    // decrease the selected value?
    if (direction == RotaryEncoder::Direction::COUNTERCLOCKWISE) {
      switch (editIndex) {
      case 0:
        numberOfDays--;
        break;
      case 1:
        if (numberOfHours == 0) {
          numberOfHours = 24;
        }
        numberOfHours--;
        break;
      case 2:
        if (numberOfMinutes == 0) {
          numberOfMinutes = 60;
        }
        numberOfMinutes--;
        break;
      }
      redraw = true; // redraw required
    }
  }

protected:
  /**
   * @brief handles if the encoder was rotated
   *
   * @param[in, out] redraw true if the content of the display must be redraw
   *
   * @return false if the current tick should be immediately skipped
   */
  bool handleEncoderClick(bool& redraw) {
    // was the button pressed?
    if (encoder->getNewClick()) {
      // select the next value
      editIndex++;
      redraw = true; // redraw required

      // check if all 3 values were updated
      if (editIndex == 3) {
        // generate the dialog message for the are you sure question:
        time_t endDate = time(NULL) +
                         ((((time_t)numberOfDays) * 24 + ((time_t)numberOfHours)) * 60 + ((time_t)numberOfMinutes)) * 60;
        tm tmBuf;
        char buf[61];
        strftime(buf, 61, "Are you sure to lock\nuntil\n%d.%m.%Y %R?", localtime_r(&endDate, &tmBuf));

        // display dialog
        switch (lcd::DialogYesNoBack(display, encoder, buf, numberOfColumns, numberOfRows)
                  .showModal(lcd::DialogYesNoBack::DialogResult::no)) {
        case lcd::DialogYesNoBack::DialogResult::yes:
          LockState::setCachedEndDate(endDate);
          views::ViewStore::activateView(views::ViewStore::SelectDisplayTimePassed);

          return false; // immediately skip tick
        case lcd::DialogYesNoBack::DialogResult::no:
          return true;
        case lcd::DialogYesNoBack::DialogResult::back:
          // go back to menu
          activatePreviousView();
          return false;
        }
      }
    }
    return true;
  }
};
} // namespace views
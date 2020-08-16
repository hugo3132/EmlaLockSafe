/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "Tools.h"

#include <SPIFFS.h>
#include <time.h>

/**
 * @brief Singleton class managing the current lock state. The class also
 * automatically saves the lock state to flash so we know what's going on after
 * a reboot even if there is no wifi.
 */
class LockState {
public:
  /**
   * @brief States for the selection how the passed time should be displayed
   */
  enum class DisplayTimePassed { no = 0, yes = 1 };

public:
  /**
   * @brief States for the selection how the time left should be displayed
   */
  enum class DisplayTimeLeft { no = 0, yes = 1, temperature = 2, timeWithPenalty = 3, timeWithRandomPenalty = 4 };

protected:
  /**
   * @brief how the passed time should be displayed
   */
  DisplayTimePassed displayTimePassed;

protected:
  /**
   * @brief how the passed time should be displayed
   */
  DisplayTimeLeft displayTimeLeft;

protected:
  /**
   * @brief start date of the current session
   */
  time_t startDate;

protected:
  /**
   * @brief end date of the current session
   */
  time_t endDate;

protected:
  /**
   * @brief chache for the end date of the session which is currently
   * configured. This value won't be saved to the flash!
   */
  time_t cachedEndDate;

protected:
  /**
   * @brief temperature string
   */
  String temperatureString;

protected:
  /**
   * @brief Constructs the lock state by trying to reading the state from flash
   */
  LockState()
    : displayTimePassed(DisplayTimePassed::yes)
    , displayTimeLeft(DisplayTimeLeft::yes)
    , startDate(0)
    , endDate(0)
    , cachedEndDate(0) {
    // load the data from the file system if available
    Tools::detachEncoderInterrupts();
    File file = SPIFFS.open("/lockSt.bin", "r");
    if (!file) {
      Tools::attachEncoderInterrupts();
      return;
    }

    // read everything to a buffer
    constexpr auto numberOfBytes = sizeof(DisplayTimePassed) + sizeof(DisplayTimeLeft) + 2 * sizeof(time_t);
    char buf[numberOfBytes];
    auto readOk = file.readBytes(buf, numberOfBytes) == numberOfBytes;
    if (readOk) {
      temperatureString = file.readString();
    }

    file.close();
    Tools::attachEncoderInterrupts();

    if (readOk) {
      // copy the data from the buffer to the actual variables
      char* src = buf;
      memcpy(&displayTimePassed, src, sizeof(DisplayTimePassed));
      src += sizeof(DisplayTimePassed);
      memcpy(&displayTimeLeft, src, sizeof(DisplayTimeLeft));
      src += sizeof(DisplayTimeLeft);
      memcpy(&startDate, src, sizeof(time_t));
      src += sizeof(time_t);
      memcpy(&endDate, src, sizeof(time_t));
    }
  }

protected:
  /**
   * @brief Saves the current state of the object to the flash
   */
  void saveData() {
    Tools::detachEncoderInterrupts();
    File file = SPIFFS.open("/lockSt.bin", "w");
    if (!file) {
      Tools::attachEncoderInterrupts();
      return;
    }

    file.write((uint8_t*)&displayTimePassed, sizeof(DisplayTimePassed));
    file.write((uint8_t*)&displayTimeLeft, sizeof(DisplayTimeLeft));
    file.write((uint8_t*)&startDate, sizeof(time_t));
    file.write((uint8_t*)&endDate, sizeof(time_t));
    file.write((uint8_t*)temperatureString.c_str(), temperatureString.length());
    file.close();
    Tools::attachEncoderInterrupts();
  }

protected:
  /**
   * @brief Get the singleton instance
   */
  static LockState& getSingleton() {
    static LockState lockState;
    return lockState;
  }

public:
  /**
   * @brief Get how the time passed should be displayed
   */
  static const DisplayTimePassed& getDisplayTimePassed() {
    return getSingleton().displayTimePassed;
  }

public:
  /**
   * @brief Set how the time passed should be displayed
   */
  static void setDisplayTimePassed(const DisplayTimePassed& displayTimePassed) {
    getSingleton().displayTimePassed = displayTimePassed;
    getSingleton().saveData();
  }

public:
  /**
   * @brief Get how the time left should be displayed
   */
  static const DisplayTimeLeft& getDisplayTimeLeft() {
    return getSingleton().displayTimeLeft;
  }

public:
  /**
   * @brief Set how the time left should be displayed
   */
  static void setDisplayTimeLeft(const DisplayTimeLeft& displayTimeLeft) {
    getSingleton().displayTimeLeft = displayTimeLeft;
    getSingleton().saveData();
  }

public:
  /**
   * @brief Get the Start Date
   */
  static const time_t& getStartDate() {
    return getSingleton().startDate;
  }

public:
  /**
   * @brief Set the Start Date
   */
  static void setStartDate(const time_t& startDate) {
    getSingleton().startDate = startDate;
    getSingleton().saveData();
  }

public:
  /**
   * @brief Get the End Date
   */
  static const time_t& getEndDate() {
    return getSingleton().endDate;
  }

public:
  /**
   * @brief Set the End Date
   */
  static void setEndDate(const time_t& endDate) {
    getSingleton().endDate = endDate;
    getSingleton().saveData();
  }

public:
  /**
   * @brief Get the Cached End Date
   */
  static const time_t& getCachedEndDate() {
    return getSingleton().cachedEndDate;
  }

public:
  /**
   * @brief Set the Cached End Date
   */
  static void setCachedEndDate(const time_t& cachedEndDate) {
    getSingleton().cachedEndDate = cachedEndDate;
  }

public:
  /**
   * @brief Get the temperature string
   */
  static const String& getTemperatureString() {
    return getSingleton().temperatureString;
  }

public:
  /**
   * @brief Set the temperature string
   */
  static void setTemperatureString(const String& temperatureString) {
    getSingleton().temperatureString = temperatureString;
    getSingleton().saveData();
  }
};
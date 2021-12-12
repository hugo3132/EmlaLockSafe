/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "Tools.h"
#include "UsedInterrupts.h"

#include <SPIFFS.h>
#include <mutex>
#include <time.h>

/**
 * @brief Singleton class managing the current lock state. The class also
 * automatically saves the lock state to flash so we know what's going on after
 * a reboot even if there is no wifi.
 *
 * Access to this class is thread-safe
 */
class LockState {
#pragma region Enums
public:
  /**
   * @brief Enum basetype used for saving to the flash
   */
  using enumBaseType = uint8_t;

public:
  /**
   * @brief Current mode of the safe
   */
  enum class Mode : enumBaseType { emlalock = 0, manual = 1 };

public:
  /**
   * @brief States for the selection how the passed time should be displayed
   */
  enum class DisplayTimePassed : enumBaseType { no = 0, yes = 1 };

public:
  /**
   * @brief States for the selection how the time left should be displayed
   */
  enum class DisplayTimeLeft : enumBaseType { no = 0, yes = 1, temperature = 2, timeWithPenalty = 3, timeWithRandomPenalty = 4 };
#pragma endregion

protected:
  /**
   * @brief mutex used to synchronize the data access
   */
  std::mutex mtx;

#pragma region Members stored on Flash
protected:
  /**
   * @brief The mode
   */
  Mode mode;

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
   * @brief temperature string
   */
  String temperatureString;

protected:
  /**
   * @brief The number of failed session
   */
  uint32_t numberOfFailedSessions;

#pragma endregion

protected:
  /**
   * @brief chache for the end date of the session which is currently
   * configured. This value won't be saved to the flash!
   */
  time_t cachedEndDate;

protected:
  /**
   * @brief end date of the current cleaning opening or 0 if not opened for
   * cleaning. This value won't be saved to the flash!
   */
  time_t cleaningEndDate;

protected:
  /**
   * @brief time of the last update
   */
  time_t lastUpdateTime;

#pragma region singleton
protected:
  /**
   * @brief Constructs the lock state by trying to reading the state from flash
   */
  LockState()
    : mode(Mode::emlalock)
    , displayTimePassed(DisplayTimePassed::yes)
    , displayTimeLeft(DisplayTimeLeft::yes)
    , startDate(0)
    , endDate(0)
    , numberOfFailedSessions(0)
    , cachedEndDate(0)
    , cleaningEndDate(0)
    , lastUpdateTime(0) {
    loadData();
  }

protected:
  /**
   * @brief Get the singleton instance
   */
  static LockState& getSingleton() {
    static LockState lockState;
    return lockState;
  }
#pragma endregion

#pragma region Flash Access Functions
protected:
  /**
   * @brief Loads the current state of the object to the flash
   */
  void loadData() {
    Serial.println("Loading lockState");
    constexpr auto numberOfBytes = sizeof(Mode) + sizeof(DisplayTimePassed) + sizeof(DisplayTimeLeft) + 2 * sizeof(time_t) +
                                   sizeof(uint32_t);
    char buf[numberOfBytes];
    bool readOk;

    // load the data from the file system if available
    UsedInterrupts::executeWithoutInterrupts([this, &readOk, &buf]() {
      File file = SPIFFS.open("/lockSt.bin", "r");
      if (!file) {
        Serial.println("Loading lockState failed");
        return;
      }

      // read everything to a buffer
      readOk = file.readBytes(buf, numberOfBytes) == numberOfBytes;
      if (readOk) {
        temperatureString = file.readString();
      }

      file.close();
    });

    if (readOk) {
      // copy the data from the buffer to the actual variables
      char* src = buf;
      memcpy(&mode, src, sizeof(Mode));
      src += sizeof(Mode);

      memcpy(&displayTimePassed, src, sizeof(DisplayTimePassed));
      src += sizeof(DisplayTimePassed);

      memcpy(&displayTimeLeft, src, sizeof(DisplayTimeLeft));
      src += sizeof(DisplayTimeLeft);

      memcpy(&startDate, src, sizeof(time_t));
      src += sizeof(time_t);

      memcpy(&endDate, src, sizeof(time_t));
      src += sizeof(time_t);

      memcpy(&numberOfFailedSessions, src, sizeof(uint32_t));
      src += sizeof(uint32_t);
    }
  }

protected:
  /**
   * @brief Saves the current state of the object to the flash
   */
  void saveData() {
    Serial.println("Saving lockState");
    UsedInterrupts::executeWithoutInterrupts([this]() {
      File file = SPIFFS.open("/lockSt.bin", "w");
      if (!file) {
        return;
      }

      file.write((uint8_t*)&mode, sizeof(DisplayTimePassed));
      file.write((uint8_t*)&displayTimePassed, sizeof(DisplayTimePassed));
      file.write((uint8_t*)&displayTimeLeft, sizeof(DisplayTimeLeft));
      file.write((uint8_t*)&startDate, sizeof(time_t));
      file.write((uint8_t*)&endDate, sizeof(time_t));
      file.write((uint8_t*)&numberOfFailedSessions, sizeof(uint32_t));
      file.write((uint8_t*)temperatureString.c_str(), temperatureString.length());
      file.close();
    });

    Serial.println("Saved lockState");
  }
#pragma endregion

#pragma region getter / setter
#pragma region mode
public:
  /**
   * @brief Get the mode
   */
  static const Mode& getMode() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().mode;
  }

public:
  /**
   * @brief Set the mode
   */
  static void setMode(const Mode& mode) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().mode != mode) {
      getSingleton().mode = mode;
      getSingleton().saveData();
    }
  }
#pragma endregion

#pragma region displayTimePassed
public:
  /**
   * @brief Get how the time passed should be displayed
   */
  static const DisplayTimePassed& getDisplayTimePassed() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().displayTimePassed;
  }

public:
  /**
   * @brief Set how the time passed should be displayed
   */
  static void setDisplayTimePassed(const DisplayTimePassed& displayTimePassed) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().displayTimePassed != displayTimePassed) {
      getSingleton().displayTimePassed = displayTimePassed;
      getSingleton().saveData();
    }
  }
#pragma endregion

#pragma region displayTimeLeft
public:
  /**
   * @brief Get how the time left should be displayed
   */
  static const DisplayTimeLeft& getDisplayTimeLeft() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().displayTimeLeft;
  }

public:
  /**
   * @brief Set how the time left should be displayed
   */
  static void setDisplayTimeLeft(const DisplayTimeLeft& displayTimeLeft) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().displayTimeLeft != displayTimeLeft) {
      getSingleton().displayTimeLeft = displayTimeLeft;
      getSingleton().saveData();
    }
  }
#pragma endregion

#pragma region startDate
public:
  /**
   * @brief Get the Start Date
   */
  static const time_t& getStartDate() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().startDate;
  }

public:
  /**
   * @brief Set the Start Date
   */
  static void setStartDate(const time_t& startDate) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().startDate != startDate) {
      getSingleton().startDate = startDate;
      getSingleton().saveData();
    }
  }
#pragma endregion

#pragma region endDate
public:
  /**
   * @brief Get the End Date
   */
  static const time_t& getEndDate() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().endDate;
  }

public:
  /**
   * @brief Set the End Date
   */
  static void setEndDate(const time_t& endDate) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().endDate != endDate) {
      getSingleton().endDate = endDate;
      getSingleton().saveData();
    }
  }
#pragma endregion

#pragma region numberOfFailedSessions
public:
  /**
   * @brief Get the number of failed sessions
   */
  static const uint32_t& getNumberOfFailedSessions() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().numberOfFailedSessions;
  }

public:
  /**
   * @brief Set the number of failed sessions
   */
  static void setNumberOfFailedSessions(const uint32_t& numberOfFailedSessions) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().numberOfFailedSessions != numberOfFailedSessions) {
      getSingleton().numberOfFailedSessions = numberOfFailedSessions;
      getSingleton().saveData();
    }
  }
#pragma endregion

#pragma region temperatureString
public:
  /**
   * @brief Get the temperature string
   */
  static const String& getTemperatureString() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().temperatureString;
  }

public:
  /**
   * @brief Set the temperature string
   */
  static void setTemperatureString(const String& temperatureString) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().temperatureString != temperatureString) {
      getSingleton().temperatureString = temperatureString;
      getSingleton().saveData();
    }
  }
#pragma endregion

#pragma region cachedEndDate
public:
  /**
   * @brief Get the Cached End Date
   */
  static const time_t& getCachedEndDate() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().cachedEndDate;
  }

public:
  /**
   * @brief Set the Cached End Date
   */
  static void setCachedEndDate(const time_t& cachedEndDate) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().cachedEndDate != cachedEndDate) {
      getSingleton().cachedEndDate = cachedEndDate;
    }
  }
#pragma endregion

#pragma region cleaningEndDate
public:
  /**
   * @brief Get the End Date of the current cleaning opening (or 0)
   */
  static const time_t& getCleaningEndDate() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().cleaningEndDate;
  }

public:
  /**
   * @brief Set the End Date of the current cleaning opening
   */
  static void setCleaningEndDate(const time_t& cleaningEndDate) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().cleaningEndDate != cleaningEndDate) {
      getSingleton().cleaningEndDate = cleaningEndDate;
    }
  }
#pragma endregion

#pragma region lastUpdateTime
public:
  /**
   * @brief Get the time of the last update over the emlalock api
   */
  static const time_t& getLastUpdateTime() {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    return getSingleton().lastUpdateTime;
  }

public:
  /**
   * @brief Set the time of the last update over the emlalock api
   */
  static void setLastUpdateTime(const time_t& lastUpdateTime) {
    std::unique_lock<std::mutex> lock(getSingleton().mtx);
    if (getSingleton().lastUpdateTime != lastUpdateTime) {
      getSingleton().lastUpdateTime = lastUpdateTime;
    }
  }
#pragma endregion

#pragma endregion
};
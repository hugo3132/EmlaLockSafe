/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "..\UsedInterrupts.h"

#include <Arduino.h>
#include <SPIFFS.h>
#include <ViewBase.h>
#include <algorithm>
#include <stdlib.h>
#include <time.h>

namespace configuration {
/**
 * @brief Static class accessing the configuration
 */
class Configuration {
public:
  /**
   * @brief Container to restrict the time when the safe can be opened.
   *
   * The safe can only be opened between start and endTime
   */
  class TimeRestrictions {
    friend class Configuration;

  public:
    /**
     * @brief The number of seconds after midnight after which the safe can be opened
     */
    uint32_t startTime;

  public:
    /**
     * @brief The number of seconds after midnight until which the safe can be opened
     */
    uint32_t endTime;

  public:
    /**
     * @brief If set, the safe can only be opened during the specified time using the normal unlock function
     */
    bool restrictUnlockTimes;

  public:
    /**
     * @brief If set, the time for hygiene openings is restricted
     */
    bool restrictHygieneOpeningTimes;

  public:
    /**
     * @brief If set, the time when the emergency key is accepted is restricted
     */
    bool restrictEmergencyKeyTimes;

  public:
    /**
     * @brief If set, the time when the configuration server can be started is restricted
     */
    bool restrictConfigurationServer;

  public:
    /**
     * @brief Construct a new Time Restrictions object
     */
    TimeRestrictions()
      : startTime(0)
      , endTime(86400)
      , restrictUnlockTimes(false)
      , restrictHygieneOpeningTimes(false)
      , restrictEmergencyKeyTimes(false)
      , restrictConfigurationServer(false) {}

  public:
    /**
     * @brief check if the current time is valid for opening the safe
     */
    bool checkTime() const {
      // compute the number of seconds of this day
      time_t currentTime = time(NULL);
      tm tmBuf;
      localtime_r(&currentTime, &tmBuf);
      uint32_t currentSeconds = tmBuf.tm_hour * 3600 + tmBuf.tm_min * 60 + tmBuf.tm_sec;

      if (startTime == endTime) {
        return true;
      }
      else if (endTime > startTime) {
        return (currentSeconds >= startTime) && (currentSeconds <= endTime);
      }
      else {
        // Midnight overflow
        return (currentSeconds >= endTime) && (currentSeconds <= startTime);
      }
    }

  protected:
    /**
     * @brief Reads the content of this class from the configuration file stream
     *
     * @param file file stream at the position where the data should be located
     */
    void readFromFile(File& file) {
      String start = file.readStringUntil('\n');
      String end = file.readStringUntil('\n');
      String restrictions = file.readStringUntil('\n');
      start.trim();
      end.trim();
      restrictions.end();
      if (start.isEmpty() || end.isEmpty() || restrictions.isEmpty()) {
        startTime = 0;
        endTime = 86400;
        restrictUnlockTimes = false;
        restrictHygieneOpeningTimes = false;
        restrictEmergencyKeyTimes = false;
        restrictConfigurationServer = false;
      }
      else {
        startTime = strtoul(start.c_str(), NULL, 0);
        endTime = strtoul(end.c_str(), NULL, 0);

        unsigned long l = strtoul(restrictions.c_str(), NULL, 0);
        restrictUnlockTimes = l & (1 << 0);
        restrictHygieneOpeningTimes = l & (1 << 1);
        restrictEmergencyKeyTimes = l & (1 << 2);
        restrictConfigurationServer = l & (1 << 3);
      }
    }

  protected:
    /**
     * @brief Writes all configuration items of this class to the file at the current stream position
     *
     * @param file file stream at the position where the data should be located
     */
    void writeToFile(File& file) {
      file.println(startTime);
      file.println(endTime);
      file.println((restrictUnlockTimes << 0) | (restrictHygieneOpeningTimes << 1) | (restrictEmergencyKeyTimes << 2) |
                   (restrictConfigurationServer << 3));
    }
  };

#pragma region Singelton
protected:
  /**
   * @brief Function providing the instance for the singleton
   */
  inline static Configuration** getInstance() {
    static Configuration* instance = nullptr;
    return &instance;
  }

public:
  /**
   * @brief Get the Singleton object
   */
  inline static Configuration& getSingleton() {
    return **getInstance();
  }

public:
  /**
   * @brief initialize the configuration object and load the configuration file
   */
  inline static Configuration& begin() {
    if (!*getInstance()) {
      *getInstance() = new Configuration();
    }
    return **getInstance();
  }
#pragma endregion

#pragma region WiFi Settings
protected:
  /**
   * @brief The SSID(name) of your wifi
   */
  String ssid;

protected:
  /**
   * @brief The password of your wifi.
   */
  String pwd;
#pragma endregion

#pragma region Emlalock settings
protected:
  /**
   * @brief The User ID extracted from the webpage settings > API
   */
  String userId;

protected:
  /**
   * @brief The API Key extracted from the webpage settings > API
   */
  String apiKey;

protected:
  /**
   * @brief The emergency key
   */
  String emergencyKey;

protected:
  /** 
   * @brief Disable support of failed session. <b>Note: </b>If selected, the safe will be locked until the last known end date before
   * a session was ended and marked as failed. Hygiene openings can no longer be triggered since the session is no longer valid!
   */
  bool disableFailedSession;
#pragma endregion

#pragma region Miscellaneous Settings
protected:
  /**
   * @brief The timezone-string (e.g. "CET-1CEST,M3.5.0,M10.5.0/3") see also
   *  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
   */
  String timezone;

protected:
  /**
   * @brief The name to the timezone-string (e.g. "Europe/Berlin")
   */
  String timezoneName;

protected:
  /**
   * @brief Timeout of display backlight in seconds
   */
  unsigned long backlightTimeOut;

protected:
  /**
   * @brief Automatically lock after the time for hygiene opening is over. If not set, the safe stays unlocked until the hygiene
   * opening is ended on the website.
   */
  bool autoLockHygieneOpeningTimeout;

protected:
  /**
   * @brief Container to restrict the time when the safe can be opened
   */
  TimeRestrictions timeRestrictions;
#pragma endregion

public:
  Configuration() {
    readConfiguration();
  }

#pragma region Getter
#pragma region WiFi Settings
public:
  /**
   * @brief get the SSID(name) of your wifi
   */
  const String& getSsid() const {
    return ssid;
  }

public:
  /**
   * @brief get the password of your wifi.
   */
  const String& getPwd() const {
    return pwd;
  }
#pragma endregion

#pragma region Emlalock settings
public:
  /**
   * @brief get the User ID extracted from the webpage settings > API
   */
  const String& getUserId() const {
    return userId;
  }

public:
  /**
   * @brief get the API Key extracted from the webpage settings > API
   */
  const String& getApiKey() const {
    return apiKey;
  }

public:
  /**
   * @brief get the emergency key of the safe
   */
  const String& getEmergencyKey() const {
    return emergencyKey;
  }

public:
  /**
   * @brief get disable support of failed session. <b>Note: </b>If selected, the safe will be locked until the last known end date before
   * a session was ended and marked as failed. Hygiene openings can no longer be triggered since the session is no longer valid!
   */
  const bool& getDisableFailedSession() const {
    return disableFailedSession;
  }

  
#pragma endregion

#pragma region Miscellaneous Settings
public:
  /**
   * @brief get the timezone-string (e.g. "CET-1CEST,M3.5.0,M10.5.0/3") see also
   *  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
   */
  const String& getTimezone() const {
    return timezone;
  }

public:
  /**
   * @brief get the name to the timezone-string (e.g. "Europe/Berlin")
   */
  const String& getTimezoneName() const {
    return timezoneName;
  }

public:
  /**
   * @brief get the timeout of display backlight in seconds
   */
  const unsigned long& getBacklightTimeOut() const {
    return backlightTimeOut;
  }

public:
  /**
   * @brief return if the safe should automatically lock after the time for hygiene opening is over. If not set, the safe stays
   * unlocked until the hygiene opening is ended on the website.
   */
  const bool& getAutoLockHygieneOpeningTimeout() const {
    return autoLockHygieneOpeningTimeout;
  }

public:
  /**
   * @brief get the timeout of display backlight in seconds
   */
  const TimeRestrictions& getTimeRestrictions() const {
    return timeRestrictions;
  }
#pragma endregion
#pragma endregion

#pragma region Setter
public:
  /**
   * @brief Set the Wifi Settings
   *
   * @param ssid new ssid
   * @param pwd  new password
   */
  void setWifiSettings(const String& ssid, const String& pwd) {
    this->ssid = ssid;
    this->pwd = pwd;
    writeConfiguration();
  }

  /**
   * @brief Set the Configuration Settings
   *
   * @param userId new Emlalock API user id
   * @param apiKey new Emlalock API key
   * @param disableFailedSession disable support of failed session. <b>Note: </b>If selected, the safe will be locked until the last known end date before
   * a session was ended and marked as failed. Hygiene openings can no longer be triggered since the session is no longer valid!
   * @param timezoneName new name of timezone
   * @param timezone new timezone string
   * @param backlightTimeOut new timeout of display backlight in seconds
   * @param autoLockHygieneOpeningTimeout Automatically lock after the time for hygiene opening is over. If not set, the safe
   * stays unlocked until the hygiene opening is ended on the website.
   * @param timeRestrictions the new time restrictions
   */
  void setConfigurationSettings(const String& userId,
                                const String& apiKey,
                                const bool& disableFailedSession,
                                const String& timezoneName,
                                const String& timezone,
                                const long& backlightTimeOut,
                                const bool& autoLockHygieneOpeningTimeout,
                                const TimeRestrictions& timeRestrictions) {
    this->userId = userId;
    this->apiKey = apiKey;
    this->disableFailedSession = disableFailedSession;

    this->timezoneName = timezoneName;
    this->timezone = timezone;
    setenv("TZ", timezone.c_str(), 1);
    tzset();

    this->backlightTimeOut = backlightTimeOut;
    lcd::ViewBase::setBacklightTimeout(backlightTimeOut * 1000);

    this->autoLockHygieneOpeningTimeout = autoLockHygieneOpeningTimeout;
    this->timeRestrictions = timeRestrictions;
    this->timeRestrictions.startTime = std::min(this->timeRestrictions.startTime, (uint32_t)86400);
    this->timeRestrictions.endTime = std::min(this->timeRestrictions.endTime, (uint32_t)86400);

    writeConfiguration();
  }

  /**
   * @brief Generates a new key and stores it to the file system.
   */
  const String& generateNewEmergencyKey() {
    std::srand((unsigned int)micros());
    for (int i = 0; i < 6; i++) {
      emergencyKey[i] = getRandomChar();
    }

    writeConfiguration();

    return emergencyKey;
  }
#pragma endregion


public:
  /** 
   * @brief Reset all values to the default values and write configuration
   */
  void restoreFactoryDefaults() {
    ssid = "";
    pwd = "";
    userId = "";
    apiKey = "";
    emergencyKey = "AAAAAA";
    disableFailedSession = false;
    timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
    timezoneName = "Europe/Berlin";
    backlightTimeOut = 15;
    autoLockHygieneOpeningTimeout = false;
    timeRestrictions = TimeRestrictions();
    writeConfiguration();
  }

protected:
  /**
   * @brief Read all configuration data from file
   */
  void readConfiguration() {
    // load everything from the SPIFFS
    UsedInterrupts::executeWithoutInterrupts([this]() {
      File file = SPIFFS.open("/configuration.txt", "r");
      if (!file) {
        Serial.println("Loading configuration.txt failed");
        return;
      }

      ssid = file.readStringUntil('\n');
      ssid.trim();
      pwd = file.readStringUntil('\n');
      pwd.trim();
      userId = file.readStringUntil('\n');
      userId.trim();
      apiKey = file.readStringUntil('\n');
      apiKey.trim();
      emergencyKey = file.readStringUntil('\n');
      emergencyKey.trim();
      if (emergencyKey.length() != 6) {
        emergencyKey = "AAAAAA";
      }
      disableFailedSession = strtol(file.readStringUntil('\n').c_str(), NULL, 0) == 1;
      timezone = file.readStringUntil('\n');
      timezone.trim();
      timezoneName = file.readStringUntil('\n');
      timezoneName.trim();

      backlightTimeOut = strtoul(file.readStringUntil('\n').c_str(), NULL, 0);
      autoLockHygieneOpeningTimeout = strtol(file.readStringUntil('\n').c_str(), NULL, 0) == 1;

      timeRestrictions.readFromFile(file);

      file.close();
      file = SPIFFS.open("/configuration.txt", "r");
      Serial.printf("configuration.txt: \n\"%s\"\n", file.readString().c_str());
    });
  }

protected:
  /**
   * @brief writes all configuration values to the SPIFFS
   */
  void writeConfiguration() {
    // load everything from the SPIFFS
    UsedInterrupts::executeWithoutInterrupts([this]() {
      File file = SPIFFS.open("/configuration.txt", "w");
      if (!file) {
        Serial.println("Loading configuration.txt failed");
        return;
      }

      file.println(ssid);
      file.println(pwd);
      file.println(userId);
      file.println(apiKey);
      file.println(emergencyKey);
      file.println((disableFailedSession) ? "1" : "0");
      file.println(timezone);
      file.println(timezoneName);
      file.println(backlightTimeOut);
      file.println((autoLockHygieneOpeningTimeout) ? "1" : "0");

      timeRestrictions.writeToFile(file);
    });
  }

protected:
  /**
   * @brief Get a random character (A-Z, 0-9)
   */
  static char getRandomChar() {
    int x = 36;
    while (x > 35) {
      // according to example of
      // https://en.cppreference.com/w/cpp/numeric/random/rand we need this to
      // have no bias....
      x = std::rand() / ((RAND_MAX + 1u) / 35);
    }

    if (x < 26) {
      return (char)x + 'A';
    }
    return (char)x - 26 + '0';
  }
};
} // namespace configuration

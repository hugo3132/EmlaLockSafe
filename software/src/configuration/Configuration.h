/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "..\UsedInterrupts.h"

#include <Arduino.h>
#include <SPIFFS.h>
#include <stdlib.h>
#include <ViewBase.h>

namespace configuration {
/**
 * @brief Static class accessing the configuration
 */
class Configuration {
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

#pragma region Emlalock API settings
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
#pragma endregion

#pragma region Hygiene Opening Settings
protected:
  /**
   * @brief Automatically lock after the time for hygiene opening is over. If not set, the safe stays unlocked until the hygiene
   * opening is ended on the website.
   */
  bool autoLockHygieneOpeningTimeout;
#pragma endregion

public:
  Configuration() {
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
      timezone = file.readStringUntil('\n');
      timezone.trim();
      timezoneName = file.readStringUntil('\n');
      timezoneName.trim();
      backlightTimeOut = strtoul(file.readStringUntil('\n').c_str(), NULL, 0);
      autoLockHygieneOpeningTimeout = strtol(file.readStringUntil('\n').c_str(), NULL, 0) == 1;

      file.close();
      file = SPIFFS.open("/configuration.txt", "r");
      Serial.printf("configuration.txt: \n\"%s\"\n", file.readString().c_str());
    });
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

#pragma region Emlalock API settings
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
#pragma endregion
#pragma region Hygiene Opening Settings
public:
  /**
   * @brief return if the safe should automatically lock after the time for hygiene opening is over. If not set, the safe stays
   * unlocked until the hygiene opening is ended on the website.
   */
  const bool& getAutoLockHygieneOpeningTimeout() const {
    return autoLockHygieneOpeningTimeout;
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
   * @param timezoneName new name of timezone
   * @param timezone new timezone string
   * @param backlightTimeOut new timeout of display backlight in seconds
   * @param autoLockHygieneOpeningTimeout Automatically lock after the time for hygiene opening is over. If not set, the safe
   * stays unlocked until the hygiene opening is ended on the website.
   */
  void setConfigurationSettings(const String& userId,
                                const String& apiKey,
                                const String& timezoneName,
                                const String& timezone,
                                const long& backlightTimeOut,
                                const bool& autoLockHygieneOpeningTimeout) {
    this->userId = userId;
    this->apiKey = apiKey;

    this->timezoneName = timezoneName;
    this->timezone = timezone;
    setenv("TZ", timezone.c_str(), 1);
    tzset();    

    this->backlightTimeOut = backlightTimeOut;
    lcd::ViewBase::setBacklightTimeout(backlightTimeOut * 1000);

    this->autoLockHygieneOpeningTimeout = autoLockHygieneOpeningTimeout;
    Serial.println((autoLockHygieneOpeningTimeout)?"1":"0");

    writeConfiguration();
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
      file.println(timezone);
      file.println(timezoneName);
      file.println(backlightTimeOut);
      file.println((autoLockHygieneOpeningTimeout)?"1":"0");
    });
  }
#pragma endregion
};
} // namespace configuration

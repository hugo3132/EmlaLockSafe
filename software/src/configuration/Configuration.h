/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "..\UsedInterrupts.h"

#include <Arduino.h>
#include <SPIFFS.h>

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

#pragma region Timezone Settings
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
    });
  }

#pragma region Getter and Setter
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
   * @brief set the SSID(name) of your wifi
   */
  void setSsid(const String& value) {
    ssid = value;
    writeConfiguration();
  }

public:
  /**
   * @brief get the password of your wifi.
   */
  const String& getPwd() const {
    return pwd;
  }

public:
  /**
   * @brief set the password of your wifi.
   */
  void setPwd(const String& value) {
    pwd = value;
    writeConfiguration();
  }

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
   * @brief set the User ID extracted from the webpage settings > API
   */
  void setUserId(const String& value) {
    userId = value;
    writeConfiguration();
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
   * @brief set the API Key extracted from the webpage settings > API
   */
  void setApiKey(const String& value) {
    apiKey = value;
    writeConfiguration();
  }
#pragma endregion

#pragma region Timezone Settings
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
   * @brief set the timezone-string (e.g. "CET-1CEST,M3.5.0,M10.5.0/3") see also
   *  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
   */
  void setTimezone(const String& value) {
    timezone = value;
    writeConfiguration();
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
   * @brief set the name to the timezone-string (e.g. "Europe/Berlin")
   */
  void setTimezoneName(const String& value) {
    timezoneName = value;
    writeConfiguration();
  }
#pragma endregion
#pragma endregion

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
    });
  }
};
} // namespace configuration
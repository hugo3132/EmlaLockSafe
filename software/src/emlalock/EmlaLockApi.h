#pragma once
#include "../config.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

namespace emlalock {
/**
 * @brief Class interfacing Arduino to the EmlaLock API
 * @see https://www.emlalock.com/apidoc/
 */
class EmlaLockApi {
public:
protected:
  class ThreadProtectedData {
  public:
    bool lockState;
    time_t startDate;
    time_t endDate;
    /**
     * @brief The time when the last connection to the EmlaLock Server was
     * established
     */
    std::chrono::system_clock::time_point lastConnectionTime;

    ThreadProtectedData()
      : lockState(true)
      , startDate(0)
      , endDate(0)
      , lastConnectionTime(std::chrono::system_clock::from_time_t(0)) {}
  };
  network_core_lib::threading::ThreadSafeTriggerVariable<ThreadProtectedData> data;

protected:
  /**
   * @brief The base address to the API
   */
  const String host = "www.emlalock.com";

protected:
  /**
   * @brief JSON parser
   */
  StaticJsonDocument<10000> jsonDocument;

public:
  /**
   * @brief Get the singleton instance of the API handler
   */
  static EmlaLockApi& getInstance() {
    static EmlaLockApi* instance = nullptr;
    if (!instance) {
      instance = new EmlaLockApi();
    }

    return *instance;
  }

protected:
  /**
   * @brief Construct a new EmlaLock Api Object. Use the singleton EmlaLock or
   * getInstance() instead of creating new objects.
   */
  EmlaLockApi()
    : data("Protected Data") {
    esp32::Thread::create("ElmaApiThread", 8192, 1, 1, *this, &EmlaLockApi::threadFunction);
    data.triggerAll();
  }

public:
  /**
   * @brief Triggers the client to reload the current state from the EmlaLock
   * server
   */
  void triggerRefresh() {
    data.triggerAll();
  }

public:
  /**
   * @brief returns if a chastity session is active
   */
  const bool isLocked() {
    auto lock = data.getUniqueLock();
    return data.v.lockState;
  }

public:
  /**
   * @brief returns the start date of the current session or 0 if it should not
   * be displayed.
   */
  const time_t getStartDate() {
    auto lock = data.getUniqueLock();
    return data.v.startDate;
  }

public:
  /**
   * @brief returns the end date of the current session or 0 if it should not be
   * displayed.
   */
  const time_t getEndDate() {
    auto lock = data.getUniqueLock();
    return data.v.endDate;
  }

public:
  /**
   * @brief returns the time when the last connection to the EmlaLock Server was
   * * established
   */
  const   std::chrono::system_clock::time_point getLastConnectionTime() {
    auto lock = data.getUniqueLock();
    return data.v.lastConnectionTime;
  }
  
protected:
  /**
   * @brief The thread functions which is communicating asynchronously with the
   * Emlalock API
   */

  void threadFunction() {
    // This thread runs forever
    while (!data.waitForTrigger(std::chrono::seconds(600), true).shutdownRequest) {
      StateManger::getInstance().operationActive = true;

      if (requestUrl(String("/info/?userid=") + USER_ID + "&apikey=" + API_KEY)) {
        auto lock = data.getUniqueLock();
        if (jsonDocument["chastitysession"].size() != 0) {
          data.v.lockState = true;
          if (jsonDocument["chastitysession"]["displaymode"]["timepassed"].as<bool>()) {
            data.v.startDate = jsonDocument["chastitysession"]["startdate"].as<time_t>();
          }
          else {
            data.v.startDate = 0;
          }
          if (jsonDocument["chastitysession"]["displaymode"]["timeleft"].as<bool>()) {
            data.v.endDate = jsonDocument["chastitysession"]["enddate"].as<time_t>();
          }
          else {
            data.v.endDate = 0;
          }
        }
        else {
          data.v.lockState = false;
          data.v.startDate = 0;
          data.v.endDate = 0;
        }
        data.v.lastConnectionTime = std::chrono::system_clock::now();
        StateManger::getInstance().lastConnectionTicks = xTaskGetTickCount();
      }
      else {
        Serial.println("Connection failed.");
      }

      StateManger::getInstance().operationActive = false;
    }
  }


protected:
  /**
   * @brief Loads the requested url and parses the json result
   *
   * @param url the URL to be loaded
   * @return true if no error occurred.
   */
  bool requestUrl(const String& url) {
    WiFiClientSecure client;

    // connect to server
    client.connect(host.c_str(), 443);
    if (!client.connected()) {
      Serial.println("Connection failed!");
      return false;
    }

    // send get request
    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: ESP8266\r\n" +
                 "Connection: Keep-Alive\r\n\r\n");

    // wait until the header is received.
    String result = "Invalid Header received.";
    while (client.connected()) {
      String line = client.readStringUntil('\n');

      if ((line.length() > 8) && (line.substring(0, 8) == "HTTP/1.1")) {
        result = line.substring(9);
      }

      if (line == "\r") {
        break;
      }
    }

    // check result
    result.trim();
    if (result != "200 OK") {
      Serial.println(String("Error Getting https://") + host + url + ": " + result);
      // read to end since we want to reuse the connection
      client.readString();
      return false;
    }

    // update json doc
    DeserializationError error = deserializeJson(jsonDocument, client);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return false;
    }

    return true;
  }
};

} // namespace emlalock

/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */

#pragma once
#include "../config.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#define ARDUINOJSON_DECODE_UNICODE 1

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Thread.h>
#include <WiFiClientSecure.h>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <mutex>

namespace emlalock {
/**
 * @brief Class interfacing Arduino to the EmlaLock API
 * @see https://www.emlalock.com/apidoc/
 */
class EmlaLockApi {
protected:
  /**
   * @brief The base address to the API
   */
  const String host = "api.emlalock.com";

protected:
  /**
   * @brief JSON parser
   */
  StaticJsonDocument<10000> jsonDocument;

protected:
  /**
   * @brief Mutex used to ensure thread safety of the API
   */
  std::mutex mtx;

protected:
  /**
   * @brief Condition variable to which can be triggered to immediately request
   * the status of the EmlaLock Session
   */
  std::condition_variable condVar;

protected:
  /**
   * @brief Helper variable to detect spurious wakeups of thread.
   */
  bool triggered;

protected:
  /**
   * @brief If true the thread requesting data from emlalock won't be started.
   */
  bool offlineMode;

public:
  /**
   * @brief Get the singleton instance of the API handler
   *
   * @param offlineMode if true the thread requesting data from emlalock won't
   * be started.
   */
  static EmlaLockApi& getSingleton(bool offlineMode = false) {
    static EmlaLockApi* instance = nullptr;
    if (!instance) {
      instance = new EmlaLockApi(offlineMode);
    }

    return *instance;
  }

protected:
  /**
   * @brief Construct a new EmlaLock Api Object. Use the singleton EmlaLock or
   * getInstance() instead of creating new objects.
   *
   * @param offlineMode if true the thread requesting data from emlalock won't
   * be started.
   */
  EmlaLockApi(bool offlineMode)
    : triggered(true)
    , offlineMode(offlineMode) {
    if (offlineMode) {
      Serial.println("Starting EmlaLock API in offline mode..");
    }
    else {
      Serial.println("Starting EmlaLock API in online mode..");
      esp32::Thread::create("ElmaApiThread", 8192, 1, 1, *this, &EmlaLockApi::threadFunction);
    }
  }

public:
  /**
   * @brief Triggers the client to reload the current state from the EmlaLock
   * server
   */
  void triggerRefresh() {
    std::unique_lock<std::mutex> lock(mtx);
    if (offlineMode) {
      LockState::setLastUpdateTime(time(NULL));
    }
    else {
      triggered = true;
      lock.unlock();
      condVar.notify_all();
    }
  }

protected:
  /**
   * @brief The thread functions which is communicating asynchronously with the
   * Emlalock API
   */
  void threadFunction() {
    // This thread runs forever
    std::unique_lock<std::mutex> lock(mtx);
    while (true) {
      // wait until the thread will be triggered or every ten minutes...
      condVar.wait_for(lock, std::chrono::seconds(600), [this]() -> bool {
        return triggered;
      }); // use lambda to avoid spurious wakeups
      triggered = false;

      // ensure we are not in the manual mode!
      if ((LockState::getEndDate() > time(NULL)) && (LockState::getMode() == LockState::Mode::manual)) {
        continue;
      }
      else {
        LockState::setMode(LockState::Mode::emlalock);
      }

      if (requestUrl(String("/info/?userid=") + USER_ID + "&apikey=" + API_KEY)) {
        uint32_t numberOfFailedSessionOld = LockState::getNumberOfFailedSessions();
        uint32_t numberOfFailedSessionNew = jsonDocument["user"]["failedsessions"].as<uint32_t>();
        if (numberOfFailedSessionOld != numberOfFailedSessionNew) {
          LockState::setNumberOfFailedSessions(numberOfFailedSessionNew);
          // Check if a session with an known end date was aborted:
          if ((LockState::getMode() == LockState::Mode::emlalock) && (LockState::getEndDate() != 0)) {
            // that's not allowed if this happened during an active
            // emlalock-session! Switch to manual mode with the last known end-time
            Serial.println("Abort rejected");
            LockState::setMode(LockState::Mode::manual);
            return;
          }
        }

        if (jsonDocument["chastitysession"].size() != 0) {
          LockState::setDisplayTimePassed(
            (LockState::DisplayTimePassed)jsonDocument["chastitysession"]["displaymode"]["timepassed"].as<int>());
          LockState::setDisplayTimeLeft(
            (LockState::DisplayTimeLeft)jsonDocument["chastitysession"]["displaymode"]["timeleft"].as<int>());

          if (LockState::getDisplayTimePassed() == LockState::DisplayTimePassed::yes) {
            LockState::setStartDate(jsonDocument["chastitysession"]["startdate"].as<time_t>());
          }
          else {
            LockState::setStartDate(0);
          }

          if (LockState::getDisplayTimeLeft() == LockState::DisplayTimeLeft::yes) {
            LockState::setEndDate(jsonDocument["chastitysession"]["enddate"].as<time_t>());
          }
          else {
            if (LockState::getDisplayTimeLeft() == LockState::DisplayTimeLeft::temperature) {
              String s = jsonDocument["chastitysession"]["enddate"].as<String>();
              s.replace("{{localization.", "");
              s.replace("}}", "");
              LockState::setTemperatureString(s);
            }
            LockState::setEndDate(std::numeric_limits<time_t>::max());
          }

          if (jsonDocument["chastitysession"]["incleaning"].as<int>() != 0) {
            LockState::setCleaningEndDate(jsonDocument["chastitysession"]["cleaningstarted"].as<time_t>() +
                                          jsonDocument["chastitysession"]["timeforcleaning"].as<time_t>());
          }
          else {
            LockState::setCleaningEndDate(0);
          }
        }
        else {
          // Session was closed on server ---> reset if the enddate was not
          // displayed...
          LockState::setEndDate(0);
        }
      }
      else {
        Serial.println("Connection failed.");
      }
      LockState::setLastUpdateTime(time(NULL));
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

    Serial.print("Host: https://");
    Serial.print(host);
    Serial.print("/");
    Serial.println(url);

    // connect to server
    client.setInsecure();
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
    String data = client.readString();
    int idx = data.indexOf('{');
    if(idx == -1) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println("Cannot find '{'!");
      return false;
    }

    data.remove(0,idx);
    DeserializationError error = deserializeJson(jsonDocument, data);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return false;
    }

    Serial.println(String("https://") + host + url + ": ");
    return true;
  }
};

} // namespace emlalock
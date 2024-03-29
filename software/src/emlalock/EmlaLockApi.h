/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../configuration/Configuration.h"
#define ARDUINOJSON_USE_LONG_LONG  1
#define ARDUINOJSON_DECODE_UNICODE 1

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
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

      if (requestUrl("/info/?userid=" + configuration::Configuration::getSingleton().getUserId() +
                     "&apikey=" + configuration::Configuration::getSingleton().getApiKey())) {
        uint32_t numberOfFailedSessionOld = LockState::getNumberOfFailedSessions();
        uint32_t numberOfFailedSessionNew = jsonDocument["user"]["failedsessions"].as<uint32_t>();
        if (numberOfFailedSessionOld != numberOfFailedSessionNew) {
          LockState::setNumberOfFailedSessions(numberOfFailedSessionNew);
          // Check if a session with an known end date was aborted:
          if ((LockState::getMode() == LockState::Mode::emlalock) && (LockState::getEndDate() != 0) && (configuration::Configuration::getSingleton().getDisableFailedSession())) {
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
            char buf[21];
            tm tmBuf;
            strftime(buf, 21, "%d.%m.%Y %T", localtime_r(&LockState::getEndDate(), &tmBuf));
            Serial.printf("End-date: %s\n", buf);
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
  bool requestUrl(String& url) {
    WiFiClientSecure client;
    client.setInsecure();
    url = String("https://") + host + url;
    Serial.println(url);

    // Your Domain name with URL path or IP address with path
    HTTPClient http;
    http.useHTTP10(true);
    http.begin(client, url);

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    String data;
    if (httpResponseCode > 0) {
      DeserializationError error = deserializeJson(jsonDocument, http.getStream());
      http.end();
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return false;
      }
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      // Free resources
      http.end();
      return false;
    }

    return true;
  }
};

} // namespace emlalock
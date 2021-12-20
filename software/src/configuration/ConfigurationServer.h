/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../UsedInterrupts.h"
#include "ConfigurationServerBase.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_PCF8574.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <list>
#include <mutex>
#include <stdlib.h>

namespace configuration {
/**
 * @brief Class implementing all functions to configure the Controller
 */
class ConfigurationServer : public ConfigurationServerBase {
#pragma region Singelton
protected:
  /**
   * @brief Function providing the instance for the singleton
   */
  inline static ConfigurationServer** getInstance() {
    static ConfigurationServer* instance = nullptr;
    return &instance;
  }

public:
  /**
   * @brief Get the Singleton object
   */
  inline static ConfigurationServer& getSingleton() {
    return **getInstance();
  }

  /**
   * @brief Create a new Configuration server
   */
  inline static ConfigurationServer& begin() {
    if (!*getInstance()) {
      *getInstance() = new ConfigurationServer();
    }
    return **getInstance();
  }

  /**
   * @brief Destroy the configuration server
   */
  inline static void end() {
    if (*getInstance()) {
      delete *getInstance();
      *getInstance() = nullptr;
    }
  }
#pragma endregion

private:
  /**
   * @brief Construct a new Configuration Server object
   */
  ConfigurationServer()
    : ConfigurationServerBase() {
    // Add files to webserver which are loaded from the file system
    addSpiffsFileToServer("/", "text/html", "/indexConfig.html");
    addSpiffsFileToServer("/zones.json", "text/json");

    // Add file to webserver returning the current settings
    server.on("/lastValues", HTTP_GET, [this](AsyncWebServerRequest* request) {
      request->send(200,
                    "text/plain",
                    Configuration::getSingleton().getUserId() + "\r\n" + Configuration::getSingleton().getApiKey() + "\r\n" +
                      Configuration::getSingleton().getTimezoneName() + "\r\n" +
                      Configuration::getSingleton().getBacklightTimeOut() + "\r\n" +
                      (Configuration::getSingleton().getAutoLockHygieneOpeningTimeout()?"true":"false"));
    });

    server.on("/saveData", HTTP_GET, [](AsyncWebServerRequest* request) {
      Configuration::getSingleton().setConfigurationSettings(getParam(request, "userId"),
                                                             getParam(request, "apiKey"),
                                                             getParam(request, "timezoneName"),
                                                             getParam(request, "timezone"),
                                                             strtoul(getParam(request, "backlightTimeOut").c_str(), NULL, 0),
                                                             getParam(request, "autoLockHygieneOpeningTimeout") == "true");
      request->send(200, "text/plain", "Configuration updated");
    });

    // Start Webserver
    server.begin();
  }


  static String getParam(AsyncWebServerRequest* request, const String& paramName) {
    auto param = request->getParam(paramName);
    if (param == nullptr) {
      Serial.println("Error getting parameter: " + paramName);
      return "";
    }
    else {
      Serial.println("Getting parameter: " + paramName + " = \"" + param->value() + "\"");
      return param->value();
    }
  }
};
} // namespace configuration
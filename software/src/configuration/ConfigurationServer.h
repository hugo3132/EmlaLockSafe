/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../UsedInterrupts.h"
#include "ConfigurationServerBase.h"
#include "config.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_PCF8574.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <list>
#include <mutex>

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
#pragma endregion

private:
  /**
   * @brief Construct a new Configuration Server object
   */
  ConfigurationServer()
    : ConfigurationServerBase() {
    // Add files to webserver which are loaded from the file system
    addSpiffsFileToServer("/", "text/html", "/indexConfig.html");

    // Add file to webserver returning the current settings
    server.on("/lastValues", HTTP_GET, [this](AsyncWebServerRequest* request) {
      request->send(200,
                    "text/plain",
                    Configuration::getSingleton().getUserId() + "\r\n" +
                    Configuration::getSingleton().getApiKey() + "\r\n" +
                    Configuration::getSingleton().getTimezoneName());
    });

    server.on("/saveData", HTTP_GET, [this](AsyncWebServerRequest* request) {
      Configuration::getSingleton().setConfigurationSettings(request->getParam("userId")->value(),
                                                             request->getParam("apiKey")->value(),
                                                             request->getParam("timezoneName")->value(),
                                                             request->getParam("timezone")->value());
      request->send(200, "text/plain", "Configuration Updated. Rebooting...");
      delay(100);
      ESP.restart();
    });

    // Start Webserver
    server.begin();
  }
};
} // namespace configuration
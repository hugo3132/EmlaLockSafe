/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "Configuration.h"
#include "ConfigurationServerBase.h"

#include <Arduino.h>
#include <Esp.h>
#include <LiquidCrystal_PCF8574.h>
#include <WiFi.h>
#include <list>
#include <mutex>

#define CONFIGURATION_SSID "SafeSetup"

namespace configuration {

/**
 * @brief Class implementing all functions to configure the Controller
 */
class WifiConfigurationServer : ConfigurationServerBase {
#pragma region Singelton
protected:
  /**
   * @brief Function providing the instance for the singleton
   */
  inline static WifiConfigurationServer** getInstance() {
    static WifiConfigurationServer* instance = nullptr;
    return &instance;
  }

public:
  /**
   * @brief Get the Singleton object
   */
  inline static WifiConfigurationServer* getSingletonPointer() {
    return *getInstance();
  }

  /**
   * @brief Create a new Configuration server
   *
   * @param display
   * @return ConfigurationServer&
   */
  inline static void begin(LiquidCrystal_PCF8574& display) {
    if (!*getInstance()) {
      *getInstance() = new WifiConfigurationServer(display);
    }
  }
#pragma endregion

#pragma region Members
protected:
  /**
   * @brief Reference to the display
   */
  LiquidCrystal_PCF8574& display;

protected:
  /**
   * @brief mutex for the protection of the data
   */
  std::mutex mtx;

protected:
  /**
   * @brief List with all found ssids
   */
  std::list<String> ssids;
#pragma endregion

private:
  /**
   * @brief Construct a new Configuration Server object
   *
   * @param display Reference to the LCD to display some status information
   */
  WifiConfigurationServer(LiquidCrystal_PCF8574& display)
    : ConfigurationServerBase()
    , display(display) {
    // open access point
    WiFi.softAP(CONFIGURATION_SSID, "");
    IPAddress IP = WiFi.softAPIP();

    // Add files to webserver which are loaded from the file system
    addSpiffsFileToServer("/", "text/html", "/indexWifi.html");

    // Add file to webserver listing all visible SSIDs
    server.on("/ssids", HTTP_GET, [this](AsyncWebServerRequest* request) {
      std::unique_lock<std::mutex> lock(mtx);
      String resp = "";
      for (const auto& ssid : ssids) {
        resp += ssid + "\r\n";
      }
      request->send(200, "text/plain", resp);
    });

    server.on("/saveData", HTTP_GET, [this](AsyncWebServerRequest* request) {
      Configuration::getSingleton().setWifiSettings(request->getParam("ssid")->value(), request->getParam("pwd")->value());
      request->send(200, "text/plain", "Configuration Updated. Rebooting...");
      ESP.restart();
    });

    // Start Webserver
    server.begin();
    
    // Update the LCD
    display.clear();
    display.setCursor(0, 0);
    display.print("Connect to WiFi:");
    display.setCursor(0, 1);
    display.print(CONFIGURATION_SSID);
    display.setCursor(0, 2);
    display.print("Open in Browser:");
    display.setCursor(0, 3);
    display.print("http://");
    display.print(IP);

    Serial.println("\n");
    Serial.println("Display: Connect to WiFi:");
    Serial.print("Display: ");
    Serial.println(CONFIGURATION_SSID);
    Serial.println("Display: Open in Browser:");
    Serial.print("Display: http://");
    Serial.print(IP);
  }

public:
  /**
   * @brief Forward of the Arduino loop function
   */
  virtual void loop() {
    static unsigned long nextScan = millis();
    if (millis() >= nextScan) {
      int n = WiFi.scanNetworks();
      {
        std::unique_lock<std::mutex> lock(mtx);
        ssids.clear();
        for (int i = 0; i < n; ++i) {
          ssids.push_back(WiFi.SSID(i));
        }
      }
      nextScan = millis() + 5000;
    }
  }
};
} // namespace configuration

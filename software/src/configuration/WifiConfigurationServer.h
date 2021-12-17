/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "Configuration.h"
#include "ConfigurationServerBase.h"

#include <Arduino.h>
#include <DNSServer.h>
#include <Esp.h>
#include <LiquidCrystal_PCF8574.h>
#include <WiFi.h>
#include <list>
#include <mutex>

#define CONFIGURATION_SSID "SafeSetup"

namespace configuration {

/**
 * @brief Class implementing the configuration of the WiFi
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
   * @brief Instance of the DNS server in access point mode
   */
  DNSServer dnsServer;

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

protected:
  /**
   * @brief IP address in Access Point mode
   */
  IPAddress IP;

protected:
  /**
   * @brief Network mask in Access Point mode
   */
  IPAddress netMsk;
#pragma endregion

private:
  /**
   * @brief Construct a new Configuration Server object
   *
   * @param display Reference to the LCD to display some status information
   */
  WifiConfigurationServer(LiquidCrystal_PCF8574& display)
    : ConfigurationServerBase()
    , display(display)
    , IP(10, 0, 0, 1)
    , netMsk(255, 255, 255, 0) {
    // open access point
    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);
    WiFi.disconnect();
    WiFi.setHostname("EmlalockSafe");

    // Setup Access Point
    display.clear();
    display.setCursor(0, 0);
    display.print("Setting up WiFi");
    Serial.println("Setting up WiFi");
    if (!WiFi.softAP(CONFIGURATION_SSID)) {
      display.clear();
      display.setCursor(0, 0);
      display.print("Error starting");
      display.setCursor(0, 1);
      display.print("Access Point ");
      display.setCursor(0, 2);
      display.print(CONFIGURATION_SSID);
      Serial.print("Error starting Access Point ");
      Serial.println(CONFIGURATION_SSID);
      return;
    }
    delay(2000); // Without delay I've seen the IP address blank
    WiFi.softAPConfig(IP, IP, netMsk);
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", IP);

    // Setup Webserver
    display.clear();
    display.setCursor(0, 0);
    display.print("Setting up Webserver");
    Serial.println("Setting up Webserver");
    delay(500);
    addSpiffsFileToServer("/", "text/html", "/indexWifi.html");
    addSpiffsFileToServer("/generate_204", "text/html", "/indexWifi.html");
    addSpiffsFileToServer("/favicon.ico", "text/html", "/indexWifi.html");
    addSpiffsFileToServer("/fwlink", "text/html", "/indexWifi.html");
    addSpiffsFileToServer("/jquery-3.6.0.min.js", "text/javascript");
    addSpiffsFileToServer("/main.css", "text/css");
    addSpiffsFileToServer("/zones.json", "text/json");

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
      delay(100);
      ESP.restart();
    });

    server.onNotFound([](AsyncWebServerRequest* request) {
      // forward to index
      AsyncWebServerResponse* response = request->beginResponse(302, "text/plain", "");
      response->addHeader("Location", String("http://") + WiFi.softAPIP().toString());
      request->send(response);
    });

    server.begin(); // Web server start
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
  void loop() {
    dnsServer.processNextRequest(); // DNS

    // check the state scanning for wifis
    int16_t scanState = WiFi.scanComplete();
    if (scanState == -2) {
      // not started yet, start a new scan
      WiFi.scanNetworks(true);
    }
    else if (scanState >= 0) {
      // scan completed
      std::unique_lock<std::mutex> lock(mtx);
      ssids.clear();
      for (int i = 0; i < scanState; ++i) {
        ssids.push_back(WiFi.SSID(i));
      }
      WiFi.scanNetworks(true);
    }
  }
};
} // namespace configuration

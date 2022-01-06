/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../UsedInterrupts.h"
#include "../views/ViewStore.h"
#include "ConfigurationServerBase.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_PCF8574.h>
#include <RotaryEncoder.h>
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

public:
  /**
   * @brief Create a new Configuration server
   *
   * @param display Reference to the display
   * @param encoder Reference to the encoder
   */
  inline static ConfigurationServer& begin(LiquidCrystal_PCF8574& display, RotaryEncoder& encoder) {
    if (!*getInstance()) {
      *getInstance() = new ConfigurationServer(display, encoder);
    }
    return **getInstance();
  }
#pragma endregion

protected:
  /**
   * @brief Websocket to remote control the display
   */
  AsyncWebSocket remoteControlWebSocket;

protected:
  /**
   * @brief Reference to the display
   */
  LiquidCrystal_PCF8574& display;

protected:
  /**
   * @brief Refernce to the encoder
   */
  RotaryEncoder& encoder;

private:
  /**
   * @brief Construct a new Configuration Server object
   *
   * @param display Reference to the display
   * @param encoder Reference to the encoder
   */
  ConfigurationServer(LiquidCrystal_PCF8574& display, RotaryEncoder& encoder)
    : ConfigurationServerBase()
    , remoteControlWebSocket("/ws")
    , display(display)
    , encoder(encoder) {
    // Add files to webserver which are loaded from the file system
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      UsedInterrupts::executeWithoutInterrupts([request]() {
        if (isConfigurationAllowed()) {
          request->send(SPIFFS, "/indexConfig.html", "text/html");
        }
        else {
          request->send(SPIFFS, "/indexRemote.html", "text/html");
        }
      });
    });
    addSpiffsFileToServer("/zones.json", "text/json");
    addSpiffsFileToServer("/moment.js", "text/javascript");
    addSpiffsFileToServer("/dotmatrix.js", "text/javascript");
    addSpiffsFileToServer("/remoteLcd.js", "text/javascript");
    addSpiffsFileToServer("/rotate.svg", "image/svg+xml");

    // Add file to webserver returning the current settings
    server.on("/lastValues", HTTP_GET, [this](AsyncWebServerRequest* request) {
      onGetLastValues(request);
    });

    // Handler to save the configuration
    server.on("/saveData", HTTP_GET, [this](AsyncWebServerRequest* request) {
      onSaveData(request);
    });

    // Handler to generate a new emergency key
    server.on("/generateNewKey", HTTP_GET, [this](AsyncWebServerRequest* request) {
      onGenerateNewKey(request);
    });

    // Handler of the websocket for the remote control of the safe
    remoteControlWebSocket.onEvent(
      [this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
        onRemoteControlWebSocketEvent(client, type, arg, data, len);
      });
    server.addHandler(&remoteControlWebSocket);

    // Start Webserver
    server.begin();
  }

protected:
  /**
   * @brief Extract a passed parameter value from a HTTP GET request
   */
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

protected:
  /**
   * @brief Quick check if the ConfigurationServerView is active --> changing the configuration is allowed
   */
  static inline bool isConfigurationAllowed() {
    return views::ViewStore::getView(views::ViewStore::ConfigurationServerView) == lcd::ViewBase::getCurrentView();
  }

protected:
  /**
   * @brief Event handler called if the current configuration should be sent to the browser
   *
   * @param request the request received from the browser
   */
  void onGetLastValues(AsyncWebServerRequest* request) {
    if (isConfigurationAllowed()) {
      String response = "";

      response += Configuration::getSingleton().getUserId();
      response += "\r\n";
      response += Configuration::getSingleton().getApiKey();
      response += "\r\n";
      response += Configuration::getSingleton().getEmergencyKey();
      response += "\r\n";
      response += Configuration::getSingleton().getDisableFailedSession() ? "true" : "false";
      response += "\r\n";
      response += Configuration::getSingleton().getTimezoneName();
      response += "\r\n";
      response += Configuration::getSingleton().getBacklightTimeOut();
      response += "\r\n";
      response += Configuration::getSingleton().getAutoLockHygieneOpeningTimeout() ? "true" : "false";
      response += "\r\n";
      response += Configuration::getSingleton().getTimeRestrictions().startTime;
      response += "\r\n";
      response += Configuration::getSingleton().getTimeRestrictions().endTime;
      response += "\r\n";
      response += Configuration::getSingleton().getTimeRestrictions().restrictUnlockTimes ? "true" : "false";
      response += "\r\n";
      response += Configuration::getSingleton().getTimeRestrictions().restrictHygieneOpeningTimes ? "true" : "false";
      response += "\r\n";
      response += Configuration::getSingleton().getTimeRestrictions().restrictEmergencyKeyTimes ? "true" : "false";
      response += "\r\n";
      response += Configuration::getSingleton().getTimeRestrictions().restrictConfigurationServer ? "true" : "false";
      response += "\r\n";

      request->send(200, "text/plain", response);
    }
    else {
      request->send(404, "text/plain", "Not Found!");
    }
  }

protected:
  /**
   * @brief Event handler called if new configuration data should be saved
   *
   * @param request the request received from the browser
   */
  void onSaveData(AsyncWebServerRequest* request) {
    if (isConfigurationAllowed()) {
      Configuration::TimeRestrictions restrictions;
      restrictions.startTime = strtoul(getParam(request, "timeRestrictionsStartTime").c_str(), NULL, 0);
      restrictions.endTime = strtoul(getParam(request, "timeRestrictionsEndTime").c_str(), NULL, 0);
      restrictions.restrictUnlockTimes = getParam(request, "timeRestrictionsRestrictUnlockTimes") == "true";
      restrictions.restrictHygieneOpeningTimes = getParam(request, "timeRestrictionsRestrictHygieneOpeningTimes") == "true";
      restrictions.restrictEmergencyKeyTimes = getParam(request, "timeRestrictionsRestrictEmergencyKeyTimes") == "true";
      restrictions.restrictConfigurationServer = getParam(request, "timeRestrictionsRestrictConfigurationServer") == "true";

      Configuration::getSingleton().setConfigurationSettings(getParam(request, "userId"),
                                                             getParam(request, "apiKey"),
                                                             getParam(request, "disableFailedSession") == "true",
                                                             getParam(request, "timezoneName"),
                                                             getParam(request, "timezone"),
                                                             strtoul(getParam(request, "backlightTimeOut").c_str(), NULL, 0),
                                                             getParam(request, "autoLockHygieneOpeningTimeout") == "true",
                                                             restrictions);
      request->send(200, "text/plain", "Configuration updated");
    }
    else {
      request->send(404, "text/plain", "Not Found!");
    }
  }

protected:
  /**
   * @brief Event handler called if a new emergency key should be generated
   *
   * @param request the request received from the browser
   */
  void onGenerateNewKey(AsyncWebServerRequest* request) {
    if (isConfigurationAllowed()) {
      request->send(200, "text/plain", Configuration::getSingleton().generateNewEmergencyKey());
    }
    else {
      request->send(404, "text/plain", "Not Found!");
    }
  }

protected:
  /**
   * @brief Event handler for the WebSocket connection which is used to remote control the safe
   * 
   * @param client the client which triggered the event
   * @param type the event type
   * @param arg the information about received data
   * @param data the received data
   * @param len the length of the received data
   */
   
  void onRemoteControlWebSocketEvent(AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("Websocket client connection received");
    }
    else if (type == WS_EVT_DATA) {
      AwsFrameInfo* info = (AwsFrameInfo*)arg;
      if ((info->opcode == WS_TEXT) && (len == 1)) {
        if (data[0] == 'L') {
          String response = display.getCompleteContent();
          response += isConfigurationAllowed() ? "true\n" : "false\n";
          response += lcd::ViewBase::isBacklightOn() ? "true\n" : "false\n";
          client->text(response);
        }
        else if (data[0] == 'A') {
          encoder.injectClockwiseRotation();
        }
        else if (data[0] == 'B') {
          encoder.injectClick();
        }
        else if (data[0] == 'C') {
          encoder.injectCounterClockwiseRotation();
        }
      }
    }
    else if (type == WS_EVT_DISCONNECT) {
      Serial.println("Client disconnected");
    }
    else {
      Serial.println("else");
    }
  }
};
} // namespace configuration
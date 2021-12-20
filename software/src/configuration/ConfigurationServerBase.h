/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../UsedInterrupts.h"

#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

namespace configuration {
/**
 * @brief Base class for configuring the controller over WiFi
 */
class ConfigurationServerBase {
protected:
  /**
   * @brief The used asynchronous webserver
   */
  AsyncWebServer server;

protected:
  /**
   * @brief Construct a new Configuration Server object
   */
  ConfigurationServerBase()
    : server(80) {
    // Add files to webserver which are loaded from the file system
    addSpiffsFileToServer("/jquery-3.6.0.min.js", "text/javascript");
    addSpiffsFileToServer("/main.css", "text/css");
  }

protected:
  /**
   * @brief Adds the file from the SPIFFS with the filename to the webserver
   * under the given path.
   *
   * @param path path under which the file should be reachable
   * @param contentType type of the data
   * @param filename optional, path on the SPIFFS. Must only be set if different to path
   */
  void addSpiffsFileToServer(const char* path, String contentType, const char* filename = nullptr) {
    if (!filename) {
      filename = path;
    }
    server.on(path, HTTP_GET, [contentType, filename](AsyncWebServerRequest* request) {
      UsedInterrupts::executeWithoutInterrupts([request, contentType, filename]() {
        request->send(SPIFFS, filename, contentType);
      });
    });
  }
};
} // namespace configuration

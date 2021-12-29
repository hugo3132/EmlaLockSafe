/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../configuration/HardwareConfiguration.h"

#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <ViewBase.h>

namespace views {
/**
 * @brief View displayed while updating the certificates
 */
class UpdateCertificatesView : public lcd::ViewBase {
protected:
  /**
   * @brief Reference of the ssl client used for the communication
   */
  WiFiClientSecure& sslClient;

protected:
  /**
   * @brief Reference of the certificate store
   */
  BearSSL::CertStore certStore;

  String lastErrorMessage;

protected:
  int numberOfLoadedCertificates;

public:
  /**
   * @brief Construct a vie object
   *
   * @param display pointer to the LCD instance
   * @param sslClient reference of the ssl client used for the communication
   * @param certStore reference of the certificate store
   */
  UpdateCertificatesView(LiquidCrystal_PCF8574* display, WiFiClientSecure& sslClient, BearSSL::CertStore& certStore)
    : lcd::ViewBase(display)
    , sslClient(sslClient)
    , certStore(certStore) {}

public:
  /**
   * @brief Copy constructor - not available
   */
  UpdateCertificatesView(const UpdateCertificatesView& other) = delete;

public:
  /**
   * @brief Move constructor - not available
   */
  UpdateCertificatesView(UpdateCertificatesView&& other) noexcept = delete;

public:
  int begin() {
    numberOfLoadedCertificates = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
    if (numberOfLoadedCertificates == 0) {
      // no SSL certificates available
      Serial.println("No SSL Certificates found in Flash.");
      sslClient.setInsecure();
    }
    else {
      Serial.print(numberOfLoadedCertificates);
      Serial.println(" SSL Certificates found in Flash.");
      sslClient.setCertStore(&certStore);
    }

    return numberOfLoadedCertificates;
  }

protected:
  /**
   * @brief called as soon as the view becomes active
   */
  virtual void activate() {
    begin();
    display->clear();
    display->setCursor(0, 0);
    display->print("Updating SSL Certs");

    // download the csv file to the file-system
    if (!downloadCertificatesAsCsv()) {
      Serial.println(lastErrorMessage);
      return;
    }
    sslClient.stopAll();

    Serial.print("Free Heap:   ");
    Serial.print(ESP.getFreeHeap());
    Serial.print("  Free Stack:  ");
    Serial.println(ESP.getFreeContStack());

    // First iteration through csv file: Count certificates
    ///////////////////////////////////////////////////////////////////////////
    File certsCsv = LittleFS.open("/certs.csv", "r");
    int numberOfCertificates = 0;
    while (true) {
      // jump to the start of the next certificate
      if (!goToStartOfNextCertificate(certsCsv)) {
        break;
      }
      numberOfCertificates++;
    }

    // check if it makes sense to regenerate the archive
    if (numberOfCertificates == 0) {
      lastErrorMessage = "No certificates found in downloaded CSV file";
      return;
    }

    Serial.print("Free Heap:   ");
    Serial.print(ESP.getFreeHeap());
    Serial.print("  Free Stack:  ");
    Serial.println(ESP.getFreeContStack());
    // go to the beginning of the CSV file to start parsing
    certsCsv.seek(0);

    // Second iteration through csv file: Convert to DER and save as archive
    ///////////////////////////////////////////////////////////////////////////
    display->clear();
    display->setCursor(0, 0);
    display->print("Updating SSL Certs");
    display->setCursor(0, 1);
    display->print("Converting:");
    display->setCursor(0, 2);
    display->print("Certificate 0/");
    display->print(numberOfCertificates);

    File certsAr = LittleFS.open("/certs.tmp", "w");
    certsAr.write("!<arch>\n");

    for (int certificateCounter = 1; certificateCounter <= numberOfCertificates; certificateCounter++) {
      display->setCursor(12, 2);
      display->print(certificateCounter);
      display->print("/");
      display->print(numberOfCertificates);

      if (!goToStartOfNextCertificate(certsCsv)) {
        break;
      }
      if (!convertCertificate(certsCsv, certsAr)) {
        break;
      }
      yield();
    }

    certsAr.close();
    certsCsv.close();
    Serial.print("Free Heap:   ");
    Serial.print(ESP.getFreeHeap());
    Serial.print("  Free Stack:  ");
    Serial.println(ESP.getFreeContStack());

    display->clear();
    display->setCursor(0, 0);
    display->print("Updating SSL Certs");
    display->setCursor(0, 1);
    display->print("Verifying...");

    Serial.print("Free Heap:   ");
    Serial.print(ESP.getFreeHeap());
    Serial.print("  Free Stack:  ");
    Serial.println(ESP.getFreeContStack());
    auto numberOfLoadedCertificates = certStore.initCertStore(LittleFS, PSTR("/certst.idx"), PSTR("/certs.tmp"));
    if (numberOfLoadedCertificates != 0) {
      if (LittleFS.exists("/certs.ar")) {
        LittleFS.remove("/certs.ar");
      }
      if (LittleFS.exists("/certs.idx")) {
        LittleFS.remove("/certs.idx");
      }
      if (LittleFS.exists("/certst.idx")) {
        LittleFS.remove("/certst.idx");
      }
      LittleFS.rename("/certs.tmp", "/certs.ar");
    }

    display->setCursor(0, 2);
    display->print("OK. Going to Reboot");

    delay(2000);
    ESP.restart();
  }

protected:
  /**
   * @brief Shows the animation while waiting until the callback returns false
   *
   * @param stillActive callback function checking if the animation should be
   * shown
   */
  void waitAnimation(std::function<bool()> stillActive) {
    while (stillActive()) {
      for (int i = 0; (i < 20) && stillActive(); i++) {
        if (i == 0) {
          display->setCursor(19, 3);
          display->print(' ');
          display->setCursor(i, 3);
          display->print('*');
        }
        else {
          display->setCursor(i - 1, 3);
          display->print(" *");
        }

        delay(100);
      }
    }
  }

protected:
  bool initializeConnection(const String& host, const String& url) {
    // try to connect to the server
    if (!sslClient.connect(host, 443)) {
      lastErrorMessage = "Connection to https://" + host + "/" + url + " failed.";
      return false;
    }
    yield();

    // send get request
    sslClient.print("GET /" + url +
                    " HTTP/1.1\r\n"
                    "Host: " +
                    host +
                    "\r\n"
                    "User-Agent: ESP8266\r\n"
                    "Connection: Keep-Alive\r\n\r\n");
    yield();

    // wait until the header is received.
    String result = "Invalid Header received.";
    while (sslClient.connected()) {
      String line = sslClient.readStringUntil('\n');

      if ((line.length() > 8) && (line.substring(0, 8) == "HTTP/1.1")) {
        result = line.substring(9);
      }

      if (line == "\r") {
        break;
      }
    }
    yield();

    // check result
    result.trim();
    if (result != "200 OK") {
      lastErrorMessage = "Error Getting https://" + host + "/" + url + ": " + result;
      sslClient.stopAll();
    }
    yield();
    return true;
  }

  bool downloadCertificatesAsCsv() {
    Serial.print("Free Heap:   ");
    Serial.print(ESP.getFreeHeap());
    Serial.print("  Free Stack:  ");
    Serial.println(ESP.getFreeContStack());
    display->setCursor(0, 1);
    display->print("Connecting...");
    if (!initializeConnection("ccadb-public.secure.force.com", "mozilla/IncludedCACertificateReportPEMCSV")) {
      return false;
    }
    Serial.print("Free Heap:   ");
    Serial.print(ESP.getFreeHeap());
    Serial.print("  Free Stack:  ");
    Serial.println(ESP.getFreeContStack());

    File certsCsv = LittleFS.open("/certs.csv", "w");
    char buf[100];
    int blockCounter = 0;

    display->setCursor(0, 1);
    display->print("Downloading 0kb     ");
    while (true) {
      size_t numBytesRead = sslClient.readBytes(buf, 100);
      if (numBytesRead == 0) {
        break;
      }
      certsCsv.write(buf, numBytesRead);

      blockCounter++;
      if (blockCounter % 100 == 0) {
        display->setCursor(12, 1);
        display->print(blockCounter / 10);
        display->print("kb");
      }

      yield();
    }
    display->setCursor(0, 1);
    display->print("CSV downloaded.  ");
    display->setCursor(0, 2);
    display->print("Saving file.");
    certsCsv.close();
    Serial.print("Free Heap:   ");
    Serial.print(ESP.getFreeHeap());
    Serial.print("  Free Stack:  ");
    Serial.println(ESP.getFreeContStack());

    // cleanup client
    sslClient.stopAll();
    Serial.print("Free Heap:   ");
    Serial.print(ESP.getFreeHeap());
    Serial.print("  Free Stack:  ");
    Serial.println(ESP.getFreeContStack());

    return true;
  }

protected:
  bool goToStartOfNextCertificate(File& certsCsv) {
    const char* searchString = "-----BEGIN CERTIFICATE-----";
    const char* searchChar = searchString;

    while (searchChar != searchString + strlen(searchString)) {
      uint8_t chr;
      if (certsCsv.read(&chr, 1) != 1) {
        return false;
      }
      // check if it is part of the search string
      if (chr == *searchChar) {
        searchChar++;
      }
      else {
        searchChar = searchString;
      }
    }

    return true;
  }

protected:
  uint16_t certificateCounter = 0;

  bool convertCertificate(File& certsCsv, File& certsAr) {
    static const uint8_t base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint8_t chr;

    // write header of file
    char headerBuf[17];
    strcpy(headerBuf, "ca_000.der/     ");
    headerBuf[3] = (certificateCounter / 100) % 10 + 48;
    headerBuf[4] = (certificateCounter / 10) % 10 + 48;
    headerBuf[5] = certificateCounter % 10 + 48;
    certificateCounter++;
    certsAr.write(headerBuf, 16);
    // file modification date
    certsAr.write("1596291962  ", 12);
    // owner id
    certsAr.write("0     ", 6);
    // group ID
    certsAr.write("0     ", 6);
    // File mode (type and permission)
    certsAr.write("100666  ", 8);
    auto sizeStart = certsAr.position();
    // File size in bytes
    certsAr.write("          ", 10);
    // Ending characters
    headerBuf[0] = 0x60;
    headerBuf[1] = 0x0A;
    certsAr.write(headerBuf, 2);

    // Parse, Convert and Write the actual certificate
    uint16_t fileSize = 0;
    while (true) {
      // buffer for the converted bytes
      uint8_t buf[3] = {0, 0, 0};
      uint8_t parserInCounter = 0;
      uint8_t parserOutCounter = 0;

      // Try to get 4 base64 characters which convert into 3 binary characters
      while (parserInCounter != 4) {
        // read next character
        if (certsCsv.read(&chr, 1) != 1) {
          return false;
        }

        // check if this is part of the end certificate string
        if (chr == '-') {
          break;
        }

        // Is it a alignment character?
        if (chr == '=') {
          parserInCounter++;
          continue;
        }

        // First part of conversion
        uint8_t i;
        for (i = 0; i < 64 && chr != base64[i]; i++) {
        }

        // Shift the bits of the conversion to the correct position
        if (i != 64) {
          switch (parserInCounter) {
          case 0:
            buf[0] = i << 2;
            parserOutCounter = 1;
            break;
          case 1:
            buf[0] += i >> 4;
            buf[1] = i << 4;
            if (buf[1] != 0) {
              parserOutCounter = 2;
            }
            break;
          case 2:
            buf[1] += i >> 2;
            buf[2] = i << 6;
            parserOutCounter = 2;
            if (buf[2] != 0) {
              parserOutCounter = 3;
            }
            break;
          case 3:
            buf[2] += i;
            parserOutCounter = 3;
            break;
          }
          parserInCounter++;
        }
      }

      // output the generated data
      certsAr.write(buf, parserOutCounter);
      fileSize += parserOutCounter;

      // Check if the certificate was fully read.
      if (chr == '-') {
        break;
      }
    }

    // check if the number of bytes is odd:
    if (fileSize % 2 != 0) {
      headerBuf[0] = 0x0A;
      certsAr.write(headerBuf, 1);
    }

    // Save the file Size
    certsAr.seek(sizeStart);
    certsAr.print(fileSize);
    certsAr.seek(0, fs::SeekEnd);

    return true;
  }

public:
  /**
   * @brief called during the loop function
   *
   * @param forceRedraw if true everything should be redrawn
   */
  virtual void tick(const bool& forceRedraw) {}
}; // namespace views
} // namespace views
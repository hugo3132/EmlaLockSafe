/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */

// Enable flag if no hardware is connected
//#define HEADLESS_API_DEBUGGING

//#define DISABLE_RTC

#include "config.h"
#include "configuration/Configuration.h"
#include "configuration/WifiConfigurationServer.h"
#include "LockState.h"
#if !defined(DISABLE_RTC)
  #include "RealTimeClock.h"
#endif
#include "Tools.h"
#include "UsedInterrupts.h"
#include "emlalock/EmlaLockApi.h"
#if !defined(HEADLESS_API_DEBUGGING)
  #include "views/EmergencyEnterKeyView.h"
  #include "views/EmergencyEnterMenuView.h"
  #include "views/EmergencyMenu.h"
  #include "views/EmlalockUnlockKeyMenu.h"
  #include "views/HardwareTestView.h"
  #include "views/HygieneOpeningMenu.h"
  #include "views/LockedView.h"
  #include "views/PreferencesMenu.h"
  #include "views/SelectDisplayTimeLeft.h"
  #include "views/SelectDisplayTimePassed.h"
  #include "views/SetTimerView.h"
  #include "views/UnlockSafeView.h"
  #include "views/UnlockedMainMenu.h"
  #include "views/ViewStore.h"
  #include "views/WifiConnectingView.h"

  #include <LiquidCrystal_PCF8574.h>
  #include <RotaryEncoder.h>
#endif
#include <SPIFFS.h>
#include <Thread.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <ds3231.h>
#include <string>
#include <sys/time.h>

#if !defined(HEADLESS_API_DEBUGGING)
RotaryEncoder encoder(ENCODER_PIN_CLK, ENCODER_PIN_DT, ENCODER_SWITCH);

LiquidCrystal_PCF8574 display(LCD_ADDR); // set the LCD address to 0x27 for a 16 chars and 2 line display

views::EmergencyEnterKeyView emergencyEnterKeyView(&display, &encoder);
views::EmergencyEnterMenuView emergencyEnterMenuView(&display);
views::EmergencyMenu emergencyMenu(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::EmlalockUnlockKeyMenu emlalockUnlockKeyMenu(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::HardwareTestView hardwareTestView(&display, &encoder);
views::LockedView lockedView(&display, &encoder);
views::HygieneOpeningMenu hygieneOpeningMenu(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::PreferencesMenu preferencesMenu(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::SelectDisplayTimeLeft selectDisplayTimeLeft(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::SelectDisplayTimePassed selectDisplayTimePassed(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::SetTimerView setTimerView(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::UnlockedMainMenu unlockedMainMenu(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::UnlockSafeView unlockSafeView(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::WifiConnectingView wifiConnectingView(&display);
#endif

/**
 * @brief Callback if one of the encoder's pin are changed
 */
void ICACHE_RAM_ATTR encoderInterrupt(void) {
#if !defined(HEADLESS_API_DEBUGGING)
  encoder.tick();
#endif
}

/**
 * @brief Setup of Sketch
 */
void setup() {
  // Setup Coil output
  pinMode(COIL_PIN, OUTPUT);
  digitalWrite(COIL_PIN, SAFE_COIL_LOCKED);

  // Initialize Serial
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("Here we go!");
  Serial.setDebugOutput(true);

  // initialize file system in flash
  Serial.println("Mount SPIFFS");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    return;
  }

  // load the configuration
  configuration::Configuration::begin();

  // Set Timezone
  setenv("TZ", configuration::Configuration::getSingleton().getTimezone().c_str(), 1);
  tzset();

#if !defined(HEADLESS_API_DEBUGGING)
  // Check if LCD works
  Wire.begin();
  Wire.beginTransmission(LCD_ADDR);
  if (Wire.endTransmission() == 0) {
    Serial.println("LCD found.");
    display.begin(LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
    display.home();
    display.clear();
    display.print("Booting...");
    display.setBacklight(1);

    UsedInterrupts::attach();
  }
  else {
    Serial.println("LCD not found.");
    return;
  }

  // Do we need to configure the WiFi?
  String ssid = configuration::Configuration::getSingleton().getSsid();
  String pwd = configuration::Configuration::getSingleton().getPwd();

  Serial.printf("SSID: '%s', PWD: '%s'\n", ssid.c_str(), pwd.c_str());
  if((ssid.length() == 0) || (pwd.length() == 0)) {
    Serial.println("Start WiFi Configuration Server");
    configuration::WifiConfigurationServer::begin(display);
    return;
  }


  {
    // Add all views to the Viewstore
    using namespace views;
    ViewStore::addView(ViewStore::EmergencyEnterKeyView, emergencyEnterKeyView);
    ViewStore::addView(ViewStore::EmergencyEnterMenuView, emergencyEnterMenuView);
    ViewStore::addView(ViewStore::EmergencyMenu, emergencyMenu);
    ViewStore::addView(ViewStore::EmlalockUnlockKeyMenu, emlalockUnlockKeyMenu);
    ViewStore::addView(ViewStore::HardwareTestView, hardwareTestView);
    ViewStore::addView(ViewStore::HygieneOpeningMenu, hygieneOpeningMenu);
    ViewStore::addView(ViewStore::LockedView, lockedView);
    ViewStore::addView(ViewStore::PreferencesMenu, preferencesMenu);
    ViewStore::addView(ViewStore::SelectDisplayTimeLeft, selectDisplayTimeLeft);
    ViewStore::addView(ViewStore::SelectDisplayTimePassed, selectDisplayTimePassed);
    ViewStore::addView(ViewStore::SetTimerView, setTimerView);
    ViewStore::addView(ViewStore::UnlockedMainMenu, unlockedMainMenu);
    ViewStore::addView(ViewStore::UnlockSafeView, unlockSafeView);
    ViewStore::addView(ViewStore::WifiConnectingView, wifiConnectingView);
  }
  lcd::ViewBase::setBacklightTimeout(15000);

  // Check if we need to go to the emergency menu
  encoder.tick();
  if (encoder.getSwitchState()) {
  #if !defined(DISABLE_RTC)
    // Check if the RTC is reachable
    Wire.beginTransmission(DS3231_I2C_ADDR);
    if (Wire.endTransmission() == 0) {
      Serial.println("Real-time Clock found.");
      RealTimeClock::loadTimeFromRtc();
    }
    else {
      Serial.println("Real-time Clock not found.");
    }
  #endif

    // Show animation to keep button pressed
    views::ViewStore::activateView(views::ViewStore::EmergencyEnterMenuView);
    for (int i = 0; (i < 20 && encoder.getSwitchState()); i++) {
      lcd::ViewBase::getCurrentView()->tick(false);
      delay(100);
    }

    encoder.getNewClick(); // otherwise we have some problem that a click is pending
    if (encoder.getSwitchState()) {
      // Set API to offline mode
      emlalock::EmlaLockApi::getSingleton(true);

      // show offline menu
      views::ViewStore::activateView(views::ViewStore::EmergencyMenu);
      return;
    }
  }

  views::ViewStore::activateView(views::ViewStore::HardwareTestView);

  #if !defined(DISABLE_RTC)
  // Check if the RTC is reachable
  Wire.beginTransmission(DS3231_I2C_ADDR);
  if (Wire.endTransmission() == 0) {
    Serial.println("Real-time Clock found.");
    RealTimeClock::loadTimeFromRtc();
  }
  else {
    Serial.println("Real-time Clock not found.");
    return;
  }
  #endif

  // Start connecting to WIFI
  views::ViewStore::activateView(views::ViewStore::WifiConnectingView);

  // Check if the safe should be locked
  if (LockState::getEndDate() > time(NULL)) {
    views::ViewStore::activateView(views::ViewStore::LockedView);
  }
  else {
    views::ViewStore::activateView(views::ViewStore::UnlockedMainMenu);
  }

#else
  // initialize file system in flash
  Serial.println("Mount SPIFFS");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    return;
  }

  Serial.println("Connecting to wifi:");
  Serial.println(WIFI_SSID);

  // Start connecting
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  WiFi.setAutoReconnect(true);

  // Play an animation while the connection is established
  int counter = 0;
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
    // if we've waited for over 2.5s (25 loops), restart wifi....
    // this is required because of an bug in the ESP32 arduino core.
    delay(2500);
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    Serial.println("Connecting to wifi:");
    Serial.println(WIFI_SSID);
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    WiFi.setAutoReconnect(true);
  }

  Serial.println("Setting Timezone:");
  // configure NTP
  configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov", "time.google.com");

  Serial.print("Connected: IP ");
  Serial.println(WiFi.localIP());
  delay(2000);
#endif
  Serial.println("EndofSetup.");
}

/**
 * @brief Loop function
 */
void loop() {
  // Is the WiFi configured?
  if(configuration::WifiConfigurationServer::getSingletonPointer()) {
    // no, just wait until the configration server reboots
    configuration::WifiConfigurationServer::getSingletonPointer()->loop();
    return;
  }

#if !defined(HEADLESS_API_DEBUGGING)
  static time_t nextRtcUpdate = time(NULL) + 120;

  // start emlalock api if necessary
  emlalock::EmlaLockApi::getSingleton();

  // tick for the encoder if the interrupt didn't properly trigger
  encoder.tick();

  // check if the safe must be locked?
  if (LockState::getEndDate() > time(NULL)) {
    // check if there is a hygiene opening
    if (LockState::getCleaningEndDate() > time(NULL)) {
      // Hygiene Opening is currently active
      const lcd::ViewBase* view = lcd::ViewBase::getCurrentView();

      // go through all previous views and look if it can be traced back to the
      // hygiene menu
      while (view && (view != views::ViewStore::getView(views::ViewStore::HygieneOpeningMenu))) {
        view = view->getPreviousView();
      }

      // if the current view does not originate from the view based on the
      // hygiene menu, activate the menu
      if (view != views::ViewStore::getView(views::ViewStore::HygieneOpeningMenu)) {
        // ensure there is no back from the menu
        lcd::ViewBase::activateView(nullptr);
        views::ViewStore::activateView(views::ViewStore::HygieneOpeningMenu);
      }
    }
    else {
      // check if the locked view is active?
      if ((lcd::ViewBase::getCurrentView() != views::ViewStore::getView(views::ViewStore::LockedView)) &&
          (lcd::ViewBase::getCurrentView() != views::ViewStore::getView(views::ViewStore::EmergencyEnterMenuView)) &&
          (lcd::ViewBase::getCurrentView() != views::ViewStore::getView(views::ViewStore::EmergencyMenu)) &&
          (lcd::ViewBase::getCurrentView() != views::ViewStore::getView(views::ViewStore::EmergencyEnterKeyView)) &&
          (lcd::ViewBase::getCurrentView() != views::ViewStore::getView(views::ViewStore::WifiConnectingView))) {
        // No activate it
        views::ViewStore::activateView(views::ViewStore::LockedView);
      }
    }
  }
  else if (lcd::ViewBase::getCurrentView() == views::ViewStore::getView(views::ViewStore::LockedView)) {
    // the locked view is active but no longer required...
    emlalock::EmlaLockApi::getSingleton().triggerRefresh();
    views::ViewStore::activateView(views::ViewStore::UnlockedMainMenu);
  }

  // tick the current view
  if (lcd::ViewBase::getCurrentView()) {
    lcd::ViewBase::getCurrentView()->tick(false);
  }
  else {
    display.setCursor(0, 0);
    for (int i = 0; i < 20; i++) {
      display.print('.');
      delay(100);
    }
    display.clear();
    display.setCursor(0, 1);
    display.print("getCurrentView()");
    display.setCursor(0, 2);
    display.print("is nullptr");
  }

  #if !defined(DISABLE_RTC)
  // Check if the RTC should be updated again?
  if (time(NULL) >= nextRtcUpdate) {
    nextRtcUpdate = time(NULL) + 120;
    RealTimeClock::saveTimeToRtc();
    Serial.println("Updated Real-Time Clock.");
  }
  #endif
#else
  Serial.printf("\n\n\n\n\nTick.\n");
  // start emlalock api if necessary
  emlalock::EmlaLockApi::getSingleton().triggerRefresh();
  delay(10000);
#endif
}
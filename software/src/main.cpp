
/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */

#include "RealTimeClock.h"
#include "config.h"
#include "views/EmergencyEnterKeyView.h"
#include "views/EmergencyEnterMenuView.h"
#include "views/EmergencyMenu.h"
#include "views/EmlalockUnlockKeyMenu.h"
#include "views/HardwareTestView.h"
#include "views/LockedView.h"
#include "views/PreferencesMenu.h"
#include "views/SetTimerView.h"
#include "views/UnlockedMainMenu.h"
#include "Tools.h"
#include "views/ViewStore.h"
#include "views/WifiConnectingView.h"

#include <LiquidCrystal_PCF8574.h>
#include <SPIFFS.h>
#include <RotaryEncoder.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <ds3231.h>
#include <string>
#include <sys/time.h>

RotaryEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_SWITCH);

LiquidCrystal_PCF8574 display(LCD_ADDR); // set the LCD address to 0x27 for a 16 chars and 2 line display

views::EmergencyEnterKeyView emergencyEnterKeyView(&display, &encoder);
views::EmergencyEnterMenuView emergencyEnterMenuView(&display);
views::EmergencyMenu emergencyMenu(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::EmlalockUnlockKeyMenu emlalockUnlockKeyMenu(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::HardwareTestView hardwareTestView(&display, &encoder);
views::LockedView lockedView(&display, &encoder);
views::PreferencesMenu preferencesMenu(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::SetTimerView setTimerView(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::UnlockedMainMenu unlockedMainMenu(&display, &encoder, LCD_NUMBER_OF_COLS, LCD_NUMBER_OF_ROWS);
views::WifiConnectingView wifiConnectingView(&display);

/**
 * @brief Callback if one of the encoder's pin are changed
 */
void ICACHE_RAM_ATTR encoderInterrupt(void) {
  encoder.tick();
}

/**
 * @brief Setup of Sketch
 */
void setup() {
  // Set Timezone
  setenv("TZ", TIME_ZONE, 1);
  tzset();

  // Initialize Serial
  Serial.begin(115200);
  Serial.println();
  Serial.println("Here we go!");
  Serial.setDebugOutput(true);

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

    Tools::attachEncoderInterrupts();
  }
  else {
    Serial.println("LCD not found.");
    return;
  }

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

  {
    // Add all views to the Viewstore
    using namespace views;
    ViewStore::addView(ViewStore::EmergencyEnterKeyView, emergencyEnterKeyView);
    ViewStore::addView(ViewStore::EmergencyEnterMenuView, emergencyEnterMenuView);
    ViewStore::addView(ViewStore::EmergencyMenu, emergencyMenu);
    ViewStore::addView(ViewStore::EmlalockUnlockKeyMenu, emlalockUnlockKeyMenu);
    ViewStore::addView(ViewStore::HardwareTestView, hardwareTestView);
    ViewStore::addView(ViewStore::LockedView, lockedView);
    ViewStore::addView(ViewStore::PreferencesMenu, preferencesMenu);
    ViewStore::addView(ViewStore::SetTimerView, setTimerView);
    ViewStore::addView(ViewStore::UnlockedMainMenu, unlockedMainMenu);
    // ViewStore::addView(ViewStore::UpdateCertificatesView, updateCertificatesView);
    ViewStore::addView(ViewStore::WifiConnectingView, wifiConnectingView);
  }
  lcd::ViewBase::setBacklightTimeout(15000);

  // initialize file system in flash
  Serial.println("Mount SPIFFS");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    return;
  }

  // Setup Coil output
  pinMode(COIL_PIN, OUTPUT);
  digitalWrite(COIL_PIN, LOW);

  // Check if we need to go to the emergency menu
  encoder.tick();
  if (encoder.getSwitchState()) {
    views::ViewStore::activateView(views::ViewStore::EmergencyEnterMenuView);

    for (int i = 0; (i < 20 && encoder.getSwitchState()); i++) {
      lcd::ViewBase::getCurrentView()->tick(false);
      delay(100);
    }

    encoder.getNewClick(); // otherwise we have some problem that a click is pending
    if (encoder.getSwitchState()) {
      views::ViewStore::activateView(views::ViewStore::EmergencyMenu);
      return;
    }
  }

  // Start connecting to WIFI
  views::ViewStore::activateView(views::ViewStore::WifiConnectingView);

  // Start normal operation
  views::ViewStore::activateView(views::ViewStore::UnlockedMainMenu);
}

/**
 * @brief Loop function
 */
void loop() {
  static time_t nextRtcUpdate = time(NULL) + 120;

  // tick for the encoder if the interrupt didn't properly trigger
  encoder.tick();

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

  // Check if the RTC should be updated again?
  if (time(NULL) >= nextRtcUpdate) {
    nextRtcUpdate = time(NULL) + 120;
    RealTimeClock::saveTimeToRtc();
    Serial.println("Updated Real-Time Clock.");
  }
}
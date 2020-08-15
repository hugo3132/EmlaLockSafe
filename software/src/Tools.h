/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "config.h"

#include <ViewBase.h>
#include <WiFi.h>

void ICACHE_RAM_ATTR encoderInterrupt(void);

/**
 * @brief class providing some static functions
 */
class Tools {
public:
  /**
   * @brief tick routine updating the wifi signal strength icon
   *
   * @param display pointer to the display
   * @param forceRedraw if true the icon is redrawn
   */
  static void tickWifiSymbol(LiquidCrystal_PCF8574* display, bool forceRedraw) {
    static uint8_t lastDrawnSymbol = 0;
    if (forceRedraw) {
      lastDrawnSymbol = 0;
    }

    // get signal strength
    long rssi = WiFi.RSSI();

    if ((rssi < -80) || (rssi >= 0)) {
      if (lastDrawnSymbol != lcd::ViewBase::scWifiSignal0) {
        display->setCursor(19, 0);
        display->write(lcd::ViewBase::scWifiSignal0);
        lastDrawnSymbol = lcd::ViewBase::scWifiSignal0;
      }
    }
    else if (rssi < -70) {
      if (lastDrawnSymbol != lcd::ViewBase::scWifiSignal1) {
        display->setCursor(19, 0);
        display->write(lcd::ViewBase::scWifiSignal1);
        lastDrawnSymbol = lcd::ViewBase::scWifiSignal1;
      }
    }
    else if (rssi < -67) {
      if (lastDrawnSymbol != lcd::ViewBase::scWifiSignal2) {
        display->setCursor(19, 0);
        display->write(lcd::ViewBase::scWifiSignal2);
        lastDrawnSymbol = lcd::ViewBase::scWifiSignal2;
      }
    }
    else if (rssi < -30) {
      if (lastDrawnSymbol != lcd::ViewBase::scWifiSignal3) {
        display->setCursor(19, 0);
        display->write(lcd::ViewBase::scWifiSignal3);
        lastDrawnSymbol = lcd::ViewBase::scWifiSignal3;
      }
    }
  }

public:
  /**
   * @brief attach the encoder pins to a interrupt
   */
  static void attachEncoderInterrupts() {
    // attach interrupts of rotary encoder
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoderInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_SWITCH), encoderInterrupt, CHANGE);
  }

public:
  /**
   * @brief detach the encoder pins to a interrupt
   */
  static void detachEncoderInterrupts() {
    detachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A));
    detachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B));
    detachInterrupt(digitalPinToInterrupt(ENCODER_SWITCH));
  }
};
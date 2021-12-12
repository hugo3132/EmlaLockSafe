/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "config.h"

#include <Arduino.h>
#include <functional>

void ICACHE_RAM_ATTR encoderInterrupt(void);

/**
 * @brief Class used to globally disable enable interrupts.
 *
 * The interrupts must e.g. disabled when accessing the SPI Filesystem
 */
class UsedInterrupts {
private:
  /**
   * @brief true if at attach() was called at least once
   */
  static bool interruptsAttached;

public:
  /**
   * @brief attach the encoder pins to a interrupt
   */
  static void attach() {
    if (!interruptsAttached) {
      attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_CLK), encoderInterrupt, CHANGE);
      attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_DT), encoderInterrupt, CHANGE);
      attachInterrupt(digitalPinToInterrupt(ENCODER_SWITCH), encoderInterrupt, CHANGE);
      interruptsAttached = true;
    }
  }

public:
  /**
   * @brief detach the interrupts
   */
  static void detach() {
    if (interruptsAttached) {
      detachInterrupt(digitalPinToInterrupt(ENCODER_PIN_CLK));
      detachInterrupt(digitalPinToInterrupt(ENCODER_PIN_DT));
      detachInterrupt(digitalPinToInterrupt(ENCODER_SWITCH));
      interruptsAttached = false;
    }
  }

public:
  /**
   * @brief Execute the passed function without interrupts
   * 
   * @param f function / lambda which should be executed
   */
  static void executeWithoutInterrupts(std::function<void(void)> f) {
    if (interruptsAttached) {
      detach();
      f();
      attach();
    }
    else {
      f();
    }
  }
};
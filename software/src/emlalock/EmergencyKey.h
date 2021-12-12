/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include "../Tools.h"
#include "../UsedInterrupts.h"

#include <SPIFFS.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <functional>

namespace emlalock {
/**
 * @brief Interface to the file holding the EmergencyKey
 */
class EmergencyKey {
public:
  /**
   * @brief Read the key to the passed array + a string terminator
   *
   * @param key target array to which the key is loaded
   */
  static void getCurrentKey(char (&key)[7]) {
    bool readOk = true;

    UsedInterrupts::executeWithoutInterrupts([&readOk, &key]() {
      File file = SPIFFS.open("/emergKey.txt", "r");
      if (file) {
        readOk = file.readBytes(key, 6) == 6;
        file.close();
      }
      else {
        readOk = false;
      }
    });

    if (!readOk) {
      generateNewKey(key);
    }
    key[6] = '\0';
  }

public:
  /**
   * @brief Generates a new key and stores it to the file system. The new key  +
   * a string terminatoris returned.
   *
   * @param key target array to which the key is generated
   *
   * @returns true if the new key was written to the flash
   */
  static bool generateNewKey(char (&key)[7]) {
    // use current time as seed for random generator
    std::srand((unsigned int)micros());
    key[0] = getRandomChar();
    key[1] = getRandomChar();
    key[2] = getRandomChar();
    key[3] = getRandomChar();
    key[4] = getRandomChar();
    key[5] = getRandomChar();
    key[6] = '\0';

    return setKey(key);  }

public:
  /**
   * @brief Saves the passed key to the filesystem.
   *
   * @param key array to with the key which should be saved
   *
   * @returns true if the new key was written to the flash
   */
  static bool setKey(char (&key)[7]) {
    UsedInterrupts::detach();
    File file = SPIFFS.open("/emergKey.txt", "w");
    if (!file) {
      file.close();
      UsedInterrupts::attach();
      strcpy(key, "Error");
      return false;
    }
    if (file.write((const uint8_t*)key, 6) != 6) {
      file.close();
      UsedInterrupts::attach();
      strcpy(key, "Error");
      return false;
    }
    file.close();
    UsedInterrupts::attach();

    return true;
  }

protected:
  /**
   * @brief Get a random character (A-Z, 0-9)
   */
  static char getRandomChar() {
    int x = 36;
    while (x > 35) {
      // according to example of
      // https://en.cppreference.com/w/cpp/numeric/random/rand we need this to
      // have no bias....
      x = std::rand() / ((RAND_MAX + 1u) / 35);
    }

    if (x < 26) {
      return (char)x + 'A';
    }
    return (char)x - 26 + '0';
  }
};
} // namespace emlalock
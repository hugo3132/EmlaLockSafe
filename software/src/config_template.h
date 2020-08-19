/**
 * @file config.h
 * @brief This file contains the actual configuration for the ESP. Never commit
 * this file because it contains the ElmaLock API-Keys and your WIFI password.
 */
#pragma once

/**
 * @brief The User ID extracted from the webpage settings > API
 * @note If the ID contains " or \ you have to escape them with \" and \\.
 */
#define USER_ID "xxxxxxxxxxxxx"

/**
 * @brief The API Key extracted from the webpage settings > API
 * @note If the key contains " or \ you have to escape them with \" and \\.
 */
#define API_KEY "xxxxxxxxxxxxx"

/**
 * @brief The SSID(name) of your wifi
 * @note If the ssid contains " or \ you have to escape them with \" and \\.
 */
#define WIFI_SSID "xxxxxxxxxxxxx"

/**
 * @brief The passwprd of your wifi.
 * @note If the password contains " or \ you have to escape them with \" and \\.
 */
#define WIFI_PWD "xxxxxxxxxxxxx"

/**
 * @brief Set Timezone to Berlin for other strings see
 * https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
 */
#define TIME_ZONE "CET-1CEST,M3.5.0,M10.5.0/3"

/**
 * @brief Set Timezone to Berlin for other strings see
 * https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
 */
#define TIME_ZONE "CET-1CEST,M3.5.0,M10.5.0/3"

/**
 * @brief GPIO the coil of the safe is connected to
 */
#define COIL_PIN 15

/**
 * @brief Value of the COIL_PIN if the coil should be locked 
 */
#define SAFE_COIL_LOCKED LOW

/**
 * @brief Value of the COIL_PIN if the coil should be unlocked 
 */
#define SAFE_COIL_UNLOCKED HIGH

/**
 * @brief GPIO of the ky-040 clk pin
 */
#define ENCODER_PIN_CLK 27

/**
 * @brief GPIO of the ky-040 dt pin
 */
#define ENCODER_PIN_DT 26

/**
 * @brief GPIO of the ky-040 sw pin
 */
#define ENCODER_SWITCH 25

/**
 * @brief I2C address of the LCD
 */
#define LCD_ADDR 0x27

/**
 * @brief number of columns of the lcd
 */
#define LCD_NUMBER_OF_COLS 20

/**
 * @brief number of rows of the lcd
 */
#define LCD_NUMBER_OF_ROWS 4
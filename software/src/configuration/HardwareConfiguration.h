/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

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
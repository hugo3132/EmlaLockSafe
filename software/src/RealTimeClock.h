/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once

#include <ds3231.h>
#include <stdlib.h>
#include <sys/time.h>

/**
 * @brief Helper syncinc the SystemClock with the RTC running in UTC.
 */
class RealTimeClock {
public:
  /**
   * @brief Sets the system clock based on the RTC
   */
  static void loadTimeFromRtc() {
    char currentTimezone[100];
    char* envTimezone = getenv("TZ");

    // was the timezone set? create a copy
    if (envTimezone) {
      strcpy(currentTimezone, envTimezone);
    }

    // The DS3231 runs in UTC. Temporarily change the system to UTC as well
    setenv("TZ", "UTC0", 1);
    tzset();

    // Initialize clock
    DS3231_init(DS3231_CONTROL_INTCN);

    // Get current time
    struct ts rtcTime;
    DS3231_get(&rtcTime);

    // convert the ts to a tm struc
    tm timeBuf;
    memset(&timeBuf, 0, sizeof(tm));
    timeBuf.tm_year = rtcTime.year - 1900;
    timeBuf.tm_mon = rtcTime.mon - 1;
    timeBuf.tm_mday = rtcTime.mday;
    timeBuf.tm_hour = rtcTime.hour;
    timeBuf.tm_min = rtcTime.min;
    timeBuf.tm_sec = rtcTime.sec;
    timeBuf.tm_isdst = -1; // negative value means no daylightsaving time as required for UTC0

    // convert to timeval
    struct timeval tv;
    tv.tv_sec = mktime(&timeBuf);
    tv.tv_usec = 0;

    // set system-clock
    settimeofday(&tv, NULL);

    // was the timezone set before? Restore the original timezone
    if (envTimezone) {
      setenv("TZ", currentTimezone, 1);
      tzset();
    }
  }

  /**
   * @brief Sets the RTC time to the value of the system clock
   */
  static void saveTimeToRtc() {
    tm timeBuf;
    struct ts rtcTime;
    // convert current time to tm structure
    time_t tNow;
    time(&tNow);

    // convert to UTC time
    gmtime_r(&tNow, &timeBuf);

    // convert tm structure to RTC structure
    rtcTime.year = timeBuf.tm_year + 1900;
    rtcTime.mon = timeBuf.tm_mon + 1;
    rtcTime.mday = timeBuf.tm_mday;
    rtcTime.hour = timeBuf.tm_hour;
    rtcTime.min = timeBuf.tm_min;
    rtcTime.sec = timeBuf.tm_sec;

    // Initialize clock
    DS3231_init(DS3231_CONTROL_INTCN);

    // Set current time
    DS3231_set(rtcTime);
  }
};
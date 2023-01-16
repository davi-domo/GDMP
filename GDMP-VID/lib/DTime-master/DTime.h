#ifndef DTIME_H
#define DTIME_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define DTIME_DAYS_UNTIL(y) ({((((y) - 1) * 365UL) + (((y) - 1) / 4) - (((y) - 1) / 100) + (((y) - 1) / 400));})
#define DTIME_DAYS_BETWEEN_YEARS(y1,y2) ({(DTIME_DAYS_UNTIL(((y1) > (y2)) ? (y1) : (y2)) - DTIME_DAYS_UNTIL(((y1) > (y2)) ? (y2) : (y1)));})
#define DTIME_LEAP_YEAR(y) ({!(((y) % 4) * (!((y) % 100) + ((y) % 400)));})
#define DTIME_MONTH_DAYS(y,m) ({(31 - (((m) > 2) ? ((((m) - 1) - (5 * ((m) > 7))) % 2) : (((m) - 1) * (3 - DTIME_LEAP_YEAR(y)))));})
#define DTIME_YEAR_DAYS_UNTIL(y,m) ({uint16_t d = 0; for(uint8_t i = 1; i < (m); i++) d += DTIME_MONTH_DAYS((y), i); d;})
#define DTIME_WEEKDAY(y,m,d) ({(DTIME_DAYS_UNTIL(y) + DTIME_YEAR_DAYS_UNTIL((y), (m)) + (d)) % 7;})

class DTime {
  public:
    const uint8_t &month = _month, &weekday = _weekday, &day = _day, &hour = _hour, &minute = _minute, &second = _second;
    const uint16_t &year = _year;
    const uint32_t &timestamp = _timestamp;

    explicit DTime() {};
    explicit DTime(uint32_t t): _timestamp(t) {
      decode();
    };
    DTime(uint16_t Y, uint8_t M, uint8_t D, uint8_t h, uint8_t m, uint8_t s);

    DTime setDate(uint16_t Y, uint8_t M, uint8_t D);
    DTime setTime(uint8_t h, uint8_t m, uint8_t s);
    DTime setTimestamp(uint32_t t);
    DTime tick();

    bool isLeapYear(uint16_t Y);
    uint8_t wday(uint16_t Y, uint8_t M, uint8_t D);

  private:
    uint8_t _month = 1, _weekday = 4, _day = 1, _hour = 0, _minute = 0, _second = 0;
    uint16_t _year = 1970;
    uint32_t _timestamp = 0;

    void decode();
    void encode();
};

#endif

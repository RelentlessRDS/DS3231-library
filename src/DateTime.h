#ifndef DATETIME
#define DATETIME

#include <cstdint>
#include <string>
#include "TimeSpan.h"

#define SECONDS_FROM_1970_TO_2000 946684800 // Unix timestamp voor 1 januari 2000, 00:00:00 UTC

class DateTime {
public:
    DateTime(uint32_t secs = SECONDS_FROM_1970_TO_2000);
    DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
    DateTime(const DateTime &copy);
    DateTime(const char *date, const char *time);
    DateTime(const char *iso8601date);
    bool isValid() const;
    char *toString(char *buffer) const;

    
uint16_t year() const {return 2000U + yOffset;}
uint8_t month() const {return m;}
uint8_t day() const {return d;}
uint8_t hour() const {return hh;}
uint8_t minute() const {return mm;}
uint8_t second() const {return ss;}
uint8_t dayOfWeek() const;
uint64_t secondstime() const;
uint64_t unixtime(void) const;

enum timestampOptions {
    TIMESTAMP_FULL,
    TIMESTAMP_DATE,
    TIMESTAMP_TIME
};

std::string timestamp(timestampOptions option = TIMESTAMP_FULL) const;

DateTime operator+(const TimeSpan &span) const;
DateTime operator-(const TimeSpan &span) const;
TimeSpan operator-(const DateTime &right) const;
bool operator<(const DateTime &right) const;
bool operator>(const DateTime &right) const {return right < *this;}
bool operator<=(const DateTime &right) const {return !(*this > right);}
bool operator>=(const DateTime &right) const {return !(*this < right);}
bool operator==(const DateTime &right) const;
bool operator!=(const DateTime &right) const {return !(*this == right);}

protected:
    uint8_t yOffset; // year offset from 2000
    uint8_t m; // month
    uint8_t d; // day
    uint8_t hh; // hour
    uint8_t mm; // minute
    uint8_t ss; // second
};

#endif

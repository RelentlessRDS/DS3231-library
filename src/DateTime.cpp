#include "DateTime.h"
#include <cstring>
#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

const uint8_t daysInMonth[] PROGMEM = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30};

// converts the date to the amount of days since 1 January 2000
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d){
    if (y >= 2000U){
        y -= 2000U;
    }
    uint16_t days = d;
    for (uint8_t i = 1; i < m; i++){
        days += pgm_read_byte(daysInMonth + i - 1);
    }
    if (m > 2 && y % 4 == 0){
        days++;
    }
    return days + 365 * y + (y + 3) / 4 - 1;
}


static uint32_t time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
  return ((days * 24UL + h) * 60 + m) * 60 + s;
}

// DateTime object constructor from unixtime
DateTime::DateTime(uint32_t t) {
    t -= SECONDS_FROM_1970_TO_2000;

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOffset = 0;; yOffset++) {
        leap = yOffset % 4 == 0;
        if (days < 365U + leap){
            break;
        }
        days -= 365 + leap;
    }
    for (m = 1; m < 12; m++) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2) {
            daysPerMonth++;
        }
        if (days < daysPerMonth) {
            break;
        }
        days -= daysPerMonth;
    }
    d = days + 1;
}

// DateTime object constructor from time indicators
DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    if (year >= 2000U) {
        year -= 2000U;
    }
    yOffset = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

DateTime::DateTime(const DateTime &copy) : yOffset(copy.yOffset), m(copy.m), d(copy.d), hh(copy.hh), mm(copy.mm), ss(copy.ss) {}

static uint8_t conv2d(const char *p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9') {
        v = *p - '0';
    }
    return 10 * v + *++p - '0';
}

DateTime::DateTime(const char *date, const char *time){
    yOffset = conv2d(date + 9);
    switch (date[0]) {
        case 'J':
            m = (date[1] == 'a') ? 1 : ((date[2] == 'n') ? 6 : 7);
            break;
        case 'F':
            m = 2;
            break;
        case 'M':
            m = date[2] == 'r' ? 3 : 5;
            break;
        case 'A':
            m = date[2] == 'r' ? 4 : 8;
            break;
        case 'S':
            m = 9;
            break;
        case 'O':
            m = 10;
            break;
        case 'N':
            m = 11;
            break;
        case 'D':
            m = 12;
            break;
    }
    d = conv2d(date + 4);
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

DateTime::DateTime(const char *iso8601dateTime){
    char ref[] = "2000-01-01T00:00:00.000";
    memcpy(ref, iso8601dateTime, std::min(strlen(ref), strlen(iso8601dateTime)));
    yOffset = conv2d(ref + 2);
    m = conv2d(ref + 5);
    d = conv2d(ref + 8);
    hh = conv2d(ref + 11);
    mm = conv2d(ref + 14);
    ss = conv2d(ref + 17);
}

bool DateTime::isValid() const {
    if (yOffset >= 100) {
        return false;
    }
    DateTime other(unixtime());
    return (yOffset == other.yOffset && m == other.m && d == other.d && hh == other.hh && mm == other.mm && ss == other.ss);
}

// Function to convert the DateTime object to a readable string
char *DateTime::toString(char *buffer) const {
  for (size_t i = 0; i < strlen(buffer) - 1; i++) {
    if (buffer[i] == 'h' && buffer[i + 1] == 'h') {
        buffer[i] = '0' + hh / 10;
        buffer[i + 1] = '0' + hh % 10;
    }
    if (buffer[i] == 'm' && buffer[i + 1] == 'm') {
      buffer[i] = '0' + mm / 10;
      buffer[i + 1] = '0' + mm % 10;
    }
    if (buffer[i] == 's' && buffer[i + 1] == 's') {
      buffer[i] = '0' + ss / 10;
      buffer[i + 1] = '0' + ss % 10;
    }
    if (buffer[i] == 'D' && buffer[i + 1] == 'D' && buffer[i + 2] == 'D') {
      static PROGMEM const char day_names[] = "SunMonTueWedThuFriSat";
      const char *p = &day_names[3 * dayOfWeek()];
      buffer[i] = pgm_read_byte(p);
      buffer[i + 1] = pgm_read_byte(p + 1);
      buffer[i + 2] = pgm_read_byte(p + 2);
    } else if (buffer[i] == 'D' && buffer[i + 1] == 'D') {
      buffer[i] = '0' + d / 10;
      buffer[i + 1] = '0' + d % 10;
    }
    if (buffer[i] == 'M' && buffer[i + 1] == 'M' && buffer[i + 2] == 'M') {
      static PROGMEM const char month_names[] =
          "JanFebMarAprMayJunJulAugSepOctNovDec";
      const char *p = &month_names[3 * (m - 1)];
      buffer[i] = pgm_read_byte(p);
      buffer[i + 1] = pgm_read_byte(p + 1);
      buffer[i + 2] = pgm_read_byte(p + 2);
    } else if (buffer[i] == 'M' && buffer[i + 1] == 'M') {
      buffer[i] = '0' + m / 10;
      buffer[i + 1] = '0' + m % 10;
    }
    if (buffer[i] == 'Y' && buffer[i + 1] == 'Y' && buffer[i + 2] == 'Y' &&
        buffer[i + 3] == 'Y') {
      buffer[i] = '2';
      buffer[i + 1] = '0';
      buffer[i + 2] = '0' + (yOffset / 10) % 10;
      buffer[i + 3] = '0' + yOffset % 10;
    } else if (buffer[i] == 'Y' && buffer[i + 1] == 'Y') {
      buffer[i] = '0' + (yOffset / 10) % 10;
      buffer[i + 1] = '0' + yOffset % 10;
    }
  }
  return buffer;
}

uint8_t DateTime::dayOfWeek() const {
    uint16_t day = date2days(yOffset, m, d);
    return (day + 6) % 7; // + 6 necessary because time stamps start at 1 January 2000, which is a Saturday
}

uint64_t DateTime::unixtime(void) const {
    uint64_t t;
    uint16_t days = date2days(yOffset, m, d);
    t = time2ulong(days, hh, mm, ss);
    t += SECONDS_FROM_1970_TO_2000;
    return t;
}

uint64_t DateTime::secondstime(void) const {
  uint64_t t;
  uint16_t days = date2days(yOffset, m, d);
  t = time2ulong(days, hh, mm, ss);
  return t;
}

DateTime DateTime::operator+(const TimeSpan &span) const {
  return DateTime(unixtime() + span.totalseconds());
}

DateTime DateTime::operator-(const TimeSpan &span) const {
  return DateTime(unixtime() - span.totalseconds());
}

TimeSpan DateTime::operator-(const DateTime &right) const {
  return TimeSpan(unixtime() - right.unixtime());
}

bool DateTime::operator<(const DateTime &right) const {
  return (yOffset + 2000U < right.year() ||
          (yOffset + 2000U == right.year() &&
           (m < right.month() ||
            (m == right.month() &&
             (d < right.day() ||
              (d == right.day() &&
               (hh < right.hour() ||
                (hh == right.hour() &&
                 (mm < right.minute() ||
                  (mm == right.minute() && 
                    (ss < right.second() ||
                      (ss == right.second()))))))))))));
}

bool DateTime::operator==(const DateTime &right) const {
  return (right.year() == yOffset + 2000U && right.month() == m &&
          right.day() == d && right.hour() == hh && right.minute() == mm &&
          right.second() == ss);
}

std::string DateTime::timestamp(timestampOptions option) const {
  char buffer[30]; // large enough for any DateTime, including invalid ones

  // Generate timestamp according to opt
  switch (option) {
  case TIMESTAMP_TIME:
    // Only time
    sprintf(buffer, "%02d:%02d:%02d", hh, mm, ss);
    break;
  case TIMESTAMP_DATE:
    // Only date
    sprintf(buffer, "%u-%02d-%02d", 2000U + yOffset, m, d);
    break;
  default:
    // Full
    sprintf(buffer, "%u-%02d-%02dT%02d:%02d:%02d", 2000U + yOffset, m, d, hh, mm,
            ss);
  }
  return buffer;
}
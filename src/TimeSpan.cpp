#include "TimeSpan.h"

TimeSpan::TimeSpan(int32_t seconds) : _seconds(seconds) {}

TimeSpan::TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds) : _seconds((int32_t)days * 86400L + (int32_t)hours * 3600 + (int32_t)minutes * 60 + seconds) {}

TimeSpan::TimeSpan(const TimeSpan &copy) : _seconds(copy._seconds) {}

TimeSpan TimeSpan::operator+(const TimeSpan &right) const {
    return TimeSpan(_seconds + right._seconds);
}

TimeSpan TimeSpan::operator-(const TimeSpan &right) const {
    return TimeSpan(_seconds - right._seconds);
}
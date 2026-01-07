#include <string>
#include <ctime>
#include <cstdint>

// NTP packet structure
struct NTPPacket {
    uint8_t li_vn_mode;      // Leap indicator, Version number, Mode
    uint8_t stratum;         // Stratum level
    uint8_t poll;            // Poll interval
    uint8_t precision;       // Precision
    uint32_t root_delay;     // Root delay
    uint32_t root_dispersion;// Root dispersion
    uint32_t ref_id;         // Reference ID
    uint32_t ref_tm_s;       // Reference time (seconds)
    uint32_t ref_tm_f;       // Reference time (fraction)
    uint32_t orig_tm_s;      // Origin time (seconds)
    uint32_t orig_tm_f;      // Origin time (fraction)
    uint32_t rx_tm_s;        // Receive time (seconds)
    uint32_t rx_tm_f;        // Receive time (fraction)
    uint32_t tx_tm_s;        // Transmit time (seconds)
    uint32_t tx_tm_f;        // Transmit time (fraction)
};

// enums for the DS3231
enum Ds3231SqwPinMode {
    DS3231_OFF = 0x1C,
    DS3231_SquareWave1Hz = 0x00,
    DS3231_SquareWave1kHz = 0x08,
    DS3231_SquareWave4kHz = 0x10,
    DS3231_SquareWave8kHz = 0x18
};

enum Ds3231Alarm1Mode {
    DS3231_A1_PerSecond = 0x0F,
    DS3231_A1_Second = 0x0E,
    DS3231_A1_Minute = 0x0C,
    DS3231_A1_Hour = 0x08,
    DS3231_A1_Date = 0x00,
    DS3231_A1_Day = 0x10
};

enum Ds3231Alarm2Mode {
    DS3231_A2_PerMinute = 0x07,
    DS3231_A2_Minute = 0x06,
    DS3231_A2_Hour = 0x04,
    DS3231_A2_Date = 0x00,
    DS3231_A2_Day = 0x08
};

class DS_3231
{
public:
    DS_3231();
    ~DS_3231();

    bool begin();
    bool lostPower();
    bool sync();
    struct tm now();
    bool adjust(const struct tm &dt);

private:
    int file;

    uint8_t decToBcd(uint8_t val) { return (val / 10 * 16) + (val % 10); }
    uint8_t bcdToDec(uint8_t val) { return (val / 16 * 10) + (val % 16); }

    bool writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

    bool getNTPTime(uint32_t& timestamp);
};
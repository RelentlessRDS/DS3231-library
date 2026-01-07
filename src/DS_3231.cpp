#include "DS_3231.h"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DS3231_ADDRESS 0x68
#define DS3231_TIME 0x00
#define DS3231_ALARM1 0x07
#define DS3231_ALARM2 0x0B
#define DS3231_CONTROL 0x0E
#define DS3231_STATUSREG 0x0F
#define DS3231_TEMPERATUREREG 0x11

DS_3231::DS_3231()
{
}

DS_3231::~DS_3231()
{
  if (file >= 0)
    close(file);
}

bool DS_3231::begin()
{
  file = open("/dev/i2c-1", O_RDWR);
  if (file < 0)
  {
    std::cerr << "Failed to open I2C bus" << std::endl;
    return false;
  }

  if (ioctl(file, I2C_SLAVE, DS3231_ADDRESS) < 0)
  {
    std::cerr << "Failed to connect to DS3231" << std::endl;
    close(file);
    file = -1;
    return false;
  }

  return true;
}

bool DS_3231::lostPower()
{
    uint8_t power = readRegister(DS3231_STATUSREG);
    return power >> 7;
}

bool DS_3231::sync()
{
  uint32_t ntpTime;
  if (!getNTPTime(ntpTime))
  {
    std::cerr << "Failed to get NTP time" << std::endl;
    return false;
  }

  time_t timestamp = static_cast<time_t>(ntpTime);
  struct tm* timeinfo = localtime(&timestamp);
  if (!adjust(*timeinfo))
  {
    std::cerr << "Failed to adjust DS3231 time" << std::endl;
    return false;
  }

  return true;
}

bool DS_3231::writeRegister(uint8_t reg, uint8_t value)
{
  uint8_t buffer[2] = { reg, value };
  return (write(file, buffer, 2) == 2);
}

uint8_t DS_3231::readRegister(uint8_t reg) {
    uint8_t buffer[1] = { reg };
    return (read(file, buffer, 1));
}

struct tm DS_3231::now()
{
  struct tm td;
  uint8_t data[7];

  uint8_t startReg = DS3231_TIME;
  if (write(file, &startReg, 1) != 1)
    return td;
  if ((read(file, data, 7) != 7))
    return td;

  td.tm_sec = bcdToDec(data[0] & 0x7F);
  td.tm_min = bcdToDec(data[1] & 0x7F);
  td.tm_hour = bcdToDec(data[2] & 0x3F);
  td.tm_mday = bcdToDec(data[4] & 0x3F);
  td.tm_mon = bcdToDec(data[5] & 0x1F) - 1;
  td.tm_year = bcdToDec(data[6]) + 100;

  return td;
}

bool DS_3231::adjust(const struct tm &dt)
{
  uint8_t startReg = DS3231_TIME;
    return writeRegister(startReg, decToBcd(dt.tm_sec))
        && writeRegister(startReg + 1, decToBcd(dt.tm_min))
        && writeRegister(startReg + 2, decToBcd(dt.tm_hour))
        && writeRegister(startReg + 4, decToBcd(dt.tm_mday))
        && writeRegister(startReg + 5, decToBcd(dt.tm_mon + 1))
        && writeRegister(startReg + 6, decToBcd(dt.tm_year - 100));
}

bool DS_3231::getNTPTime(uint32_t& timestamp)
{
    const char* ntp_server = "pool.ntp.org";
    const int ntp_port = 123;
    
    // create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        std::cerr << "Failed to create socket\n";
        return false;
    }
    
    // set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    // resolve hostname
    struct hostent* server = gethostbyname(ntp_server);
    if (server == nullptr)
    {
        std::cerr << "Failed to resolve NTP server\n";
        close(sockfd);
        return false;
    }
    
    // setup server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(ntp_port);
    
    // prepare NTP request packet
    NTPPacket packet;
    memset(&packet, 0, sizeof(packet));
    packet.li_vn_mode = 0x1B; // LI=0, VN=3, Mode=3 (client)
    
    // send request
    if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Failed to send NTP request\n";
        close(sockfd);
        return false;
    }
    
    // receive response
    socklen_t serv_len = sizeof(serv_addr);
    if (recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&serv_addr, &serv_len) < 0)
    {
        std::cerr << "Failed to receive NTP response\n";
        close(sockfd);
        return false;
    }
    
    close(sockfd);
    
    // extract timestamp (seconds since 1900-01-01)
    timestamp = ntohl(packet.tx_tm_s);
    
    // convert from NTP epoch (1900) to Unix epoch (1970)
    // there are 2208988800 seconds between 1900 and 1970
    timestamp -= 2208988800UL;
    
    return true;
}
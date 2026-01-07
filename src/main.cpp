#include "DS_3231.h"
#include <iostream>
#include <chrono>
#include <thread>

// example code for RTC functions

int main(){
    DS_3231 clock; // intialize our clock
    clock.begin(); // turn the clock on

    // power check
    if (clock.lostPower()){
        std::cout << "No power to the RTC\n";
    }
    
    // sync the RTC, can comment this out
    clock.sync();

    // get current RTC time for a number of iterations
    for (int i = 0; i < 150; i++){
        struct tm current_time = clock.now();
        if (current_time.tm_sec < 10){
            if (current_time.tm_min < 10){
                std::cout << "Time: " << current_time.tm_mday << "-" << current_time.tm_mon + 1 << "-" << current_time.tm_year + 1900 << "T" << current_time.tm_hour << ":0" << current_time.tm_min << ":0" << current_time.tm_sec << std::endl;
            } else {
                std::cout << "Time: " << current_time.tm_mday << "-" << current_time.tm_mon + 1 << "-" << current_time.tm_year + 1900 << "T" << current_time.tm_hour << ":" << current_time.tm_min << ":0" << current_time.tm_sec << std::endl;
            }
        }
        else if (current_time.tm_min < 10){
            std::cout << "Time: " << current_time.tm_mday << "-" << current_time.tm_mon + 1 << "-" << current_time.tm_year + 1900 << "T" << current_time.tm_hour << ":0" << current_time.tm_min << ":" << current_time.tm_sec << std::endl;
        } else {
            std::cout << "Time: " << current_time.tm_mday << "-" << current_time.tm_mon + 1 << "-" << current_time.tm_year + 1900 << "T" << current_time.tm_hour << ":" << current_time.tm_min << ":" << current_time.tm_sec << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(997));
    }
    return 0;
}
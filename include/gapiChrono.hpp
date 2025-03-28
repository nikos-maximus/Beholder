#pragma once
#include <string>
#include <fstream>
#include <chrono>
#include <stack>

class gapiChrono
{
public:
    enum class Unit
    {
        nanoseconds,
        microseconds,
        milliseconds,
        seconds,
        minutes,
        hours
    };
    
    gapiChrono() = delete;
    gapiChrono(const std::string& outPath);
    bool Start(const std::string& section);
    void Stop(Unit unit = Unit::microseconds);

protected:
private:
    struct Section
    {
        std::string sectionName;
        std::chrono::steady_clock::time_point startTime;
    };

    std::ofstream outFile;
    std::stack<Section> sections;
};

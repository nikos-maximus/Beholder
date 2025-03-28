#include <assert.h>
#include "gapi/_private/gapiChrono.h"

gapiChrono::gapiChrono(const std::string& outPath)
    : outFile(outPath, std::ios_base::app | std::ios_base::app)
{}

bool gapiChrono::Start(const std::string& section)
{
    if (!outFile.is_open()) return false;

    sections.push( { section, std::chrono::steady_clock::now() } );

    auto numTabs = sections.size() - 1;
    for (size_t t = 0; t < numTabs; ++t)
    {
        outFile << '\t';
    }
    outFile << "Start section: " << section << std::endl;

    return true;
}

void gapiChrono::Stop(Unit unit)
{
    if (sections.empty()) return;

    const auto& s = sections.top();
    auto dTime = std::chrono::steady_clock::now() - s.startTime;

    auto numTabs = sections.size() - 1;
    for (size_t t = 0; t < numTabs; ++t)
    {
        outFile << '\t';
    }

    std::string timeValue, unitLabel;
    switch(unit)
    {
        case Unit::nanoseconds:
        {
            timeValue = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(dTime).count());
            unitLabel = "nanoseconds";
            break;
        }
        case Unit::microseconds:
        {
            timeValue = std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(dTime).count());
            unitLabel = "microseconds";
            break;
        }
        case Unit::milliseconds:
        {
            timeValue = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(dTime).count());
            unitLabel = "milliseconds";
            break;
        }
        case Unit::seconds:
        {
            timeValue = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(dTime).count());
            unitLabel = "seconds";
            break;
        }
        case Unit::minutes:
        {
            timeValue = std::to_string(std::chrono::duration_cast<std::chrono::minutes>(dTime).count());
            unitLabel = "minutes";
            break;
        }
        case Unit::hours:
        {
            timeValue = std::to_string(std::chrono::duration_cast<std::chrono::hours>(dTime).count());
            unitLabel = "hours";
            break;
        }
        default:
        {
            assert(false);
        }
    }

    outFile << "End section: " << s.sectionName << ": Time elapsed = " << timeValue << " " << unitLabel << std::endl;
    outFile.flush();
    sections.pop();
}

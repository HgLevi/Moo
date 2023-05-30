#pragma once

#include "Math/MathUtils.hpp"
#include "MooAssert.h"

#include <chrono>

namespace moo {
    constexpr size_t TimeStampSize(std::streamsize aFractionSecondsWidth = 6, bool aDate = false) noexcept
    {
        MOO_ASSERT(aFractionSecondsWidth >= 0 && aFractionSecondsWidth <= 6);

        size_t size = 0;

        if (aDate)
        {
            //-----1234567890123456789
            //-----2001-02-03 01:02:03
            size = 10 + 1 + 8; //19
        }
        else
        {
            //-----12345678
            //-----01:02:03
            size = 8;
        }

        //---------------------------------------------1234567
        //---------------------------------------------.123456
        return size + aFractionSecondsWidth == 0 ? 0 : 1 + aFractionSecondsWidth;
    }

    inline std::string TimeStampString(std::streamsize aFractionSecondsWidth = 6, bool aDate = false)
    {
        MOO_ASSERT(aFractionSecondsWidth >= 0 && aFractionSecondsWidth <= 6);

        using namespace std;
        using namespace chrono;

        stringstream ss;

        const auto now = system_clock::now();
        const auto nowInTimeT = system_clock::to_time_t(now);

        tm timeInfo;
        localtime_s(&timeInfo, &nowInTimeT);

        if (aDate)
        {
            ss << put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
        }
        else
        {
            ss << put_time(&timeInfo, "%H:%M:%S");
        }

        if (aFractionSecondsWidth > 0)
        {
            const auto subsec = now - system_clock::from_time_t(nowInTimeT);

            const streamsize microDivider = 1000000 / moo::Pow<streamsize>(10, aFractionSecondsWidth);

            ss << '.' << setfill('0') << setw(aFractionSecondsWidth);
            ss << duration_cast<std::chrono::microseconds>(subsec).count() / microDivider;
        }

        return ss.str();
    }
}

#pragma once

#include "MooWarning.h"

#include <string>

namespace std {
    template <class T>
    struct char_traits;

    template <class _Elem, class _Traits>
    class basic_ostream;

    using ostream = basic_ostream<char, char_traits<char>>;
}

namespace moo {
    struct Where;

    inline std::string ToString(Where aWhere) noexcept;

    struct Where {
        Where(const char* aFilename = nullptr, const char* aFunction = nullptr, const uint_fast32_t aLine = 0) noexcept
            : filename(aFilename), function(aFunction), line(aLine) {}

        operator bool() const noexcept
        {
            return filename || function || line;
        }

        const char* filename;
        const char* function;
        const uint_fast32_t line;

        template<class Tos>
        Tos& Print(Tos& aOs) noexcept(noexcept(std::declval<Tos>() << std::declval<std::string>()))
        {
            if (*this)
            {
                aOs << ToString(*this);
            }

            return aOs;
        }

        template<class Tos>
        Tos& Print(Tos& aOs, std::ostream& (*aEndl)(std::ostream&))
            noexcept(noexcept(std::declval<Tos>() << std::declval<std::ostream&(*)(std::ostream&)>()))
        {
            Print(aOs);

            if (*this)
            {
                aOs << aEndl;
            }

            return aOs;
        }
    };

    inline std::string ToString(Where aWhere) noexcept
    {
        try
        {
            std::string str;

            if (aWhere.filename != nullptr)
            {
                str += std::string(aWhere.filename) + ", ";
            }

            if (aWhere.function != nullptr)
            {
                str += std::string(aWhere.function) + ": ";
            }

            if (aWhere.line != 0)
            {
                str += std::to_string(aWhere.line);
            }

            return str;
        }
        catch (...)
        {
            return __FUNCSIG__ " failed";
        }
    }

#define MOO_WHERE moo::Where(__FILE__, __FUNCTION__, __LINE__)
}

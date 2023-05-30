#pragma once
#include "MooAssert.h"
#include "Where.h"

#include "Logger.h"

#include <iostream>
#include <optional>

#ifndef MOO_DONT_ASSERT
#define MOO_ASSERT_NOEXCEPT MOO_ASSERT(false)
#else
#define MOO_ASSERT_NOEXCEPT
#endif

namespace moo {

    template<class TBase>
    class NoExceptIO : private TBase {
    public:
        template<class T>
        NoExceptIO& operator<<(const T& aValue) noexcept
        {
            try
            {
                TBase::OS() << aValue;
            }
            catch (...)
            {
                MOO_ASSERT_NOEXCEPT;
            }

            return *this;
        }

        NoExceptIO& operator<<(std::ostream& (*aFunc)(std::ostream&)) noexcept
        {
            try
            {
                TBase::OS() << aFunc;
            }
            catch (...)
            {
                MOO_ASSERT_NOEXCEPT;
            }

            return *this;
        }
    };

    struct Cout { std::ostream& OS() noexcept { return std::cout; } };
    struct Cerr { std::ostream& OS() noexcept { return std::cerr; } };
    struct Clog { std::ostream& OS() noexcept { return std::clog; } };

    static inline NoExceptIO<Cout> cout_noExcept;
    static inline NoExceptIO<Cerr> cerr_noExcept;
    static inline NoExceptIO<Clog> clog_noExcept;

    inline std::string What(const std::exception& aException) noexcept
    {
        try
        {
            return aException.what() ? aException.what() : "";
        }
        catch (...)
        {
            MOO_ASSERT_NOEXCEPT;
            return "std::exception::what throwed an exception";
        }
    }

    template<class Func, class... Args>
    auto NoExcept(Func&& aFunction, Where aWhere, Args... aArgs) noexcept
    {
        using R = MOO_RETURN_TYPE(Func, aFunction, Args, aArgs);

        std::string errorMessage;

        try
        {
            return std::invoke(std::forward<Func>(aFunction), std::forward<Args>(aArgs)...);
        }
        catch (const std::exception& e)
        {
            errorMessage = "std::exception was thrown in NoExcept call: " + What(e);
        }
        catch (...)
        {
            errorMessage = "Unknown exception was thrown in NoExcept call.;";
        }

        MOO_ASSERT_NOEXCEPT;

        {
            Logger::Lock lock;
            aWhere.Print(cerr_noExcept, std::endl);
            cerr_noExcept << errorMessage << std::endl;
        }

        return R{};
    }

    template<class Func, class... Args>
    auto NoExcept(Func&& aFunction, Args... aArgs) noexcept
    {
        return NoExcept(std::forward<Func>(aFunction), Where{}, std::forward<Args>(aArgs)...);
    }

    template<class Func, class... Args>
    auto NoExceptSuccess(Func&& aFunction, Where aWhere, Args... aArgs) noexcept
    {
        using R = MOO_RETURN_TYPE(Func, aFunction, Args, aArgs);

        auto invokeFunction = [&]()
        {
            return std::invoke(std::forward<Func>(aFunction), std::forward<Args>(aArgs)...);
        };

        if constexpr (std::is_void_v<R>)
        {
            return NoExcept([&]()
                {
                    invokeFunction();
                    return true;
                }, aWhere);
        }
        else
        {
            return NoExcept([&]()
                {
                    return std::optional(invokeFunction());
                }, aWhere);
        }
    }

    template<class Func, class... Args>
    auto NoExceptSuccess(Func&& aFunction, Args... aArgs) noexcept
    {
        return NoExceptSuccess(std::forward<Func>(aFunction), Where{}, std::forward<Args>(aArgs)...);
    }
}

#define MOO_NOEXCEPT(A_FUNC_BODY) moo::NoExcept([&]() { A_FUNC_BODY; }, MOO_WHERE)
#define MOO_NOEXCEPT_WHERE(A_FUNC_BODY, A_WHERE) moo::NoExcept([&]() { A_FUNC_BODY; }, A_WHERE)

#undef MOO_ASSERT_NOEXCEPT

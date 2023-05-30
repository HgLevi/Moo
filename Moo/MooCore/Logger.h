#pragma once
#include "MooDefaults.h"

#include <string>
#include <memory>
#include <mutex>

namespace std {
    using streamsize = long long;
}

namespace moo {
    // You can use this Logger class to redirect cout/clog/cerr when you don't have a console to write to.
    // All three are redirected to the Debug Window, and clog and cerr to a log file too.
    // Each stream starts the lines with a timestamp and an indication of the type (out/log/-ERR-).
    //
    // The Logger starts working when the first instance is created and stops when the last one is destroyed.
    // Having more than one instance at a time doesn't change any behavior, it effectively works like a Singleton,
    // except that you have RAII control over initialization and destruction.
    // After destruction, the streams go back to their original targets.
    // You can have as many instantiation/destruction as you like, effectively starting/stopping the redirection,
    // or changing the output file.
    //
    // You are supposed to use Logger::Lock as a scoped lock to avoid concurrency. You will get an assertion
    // if you forget to do so, but you can disable it if you don't need locking.

    class Logger {
    public:
        Logger(const std::string& aLogPath = "") noexcept;
        ~Logger();
        MOO_DEFAULTS(Logger);

        class Lock {
        public:
            Lock() noexcept;
            ~Lock();
            MOO_DELETE_DEFAULTS(Lock);
        private:
            std::unique_lock<std::recursive_mutex> _lock;
        };

        static void DefaultLogPath(std::string aLogPath) noexcept;
        static const std::string& DefaultLogPath() noexcept;

        static void AssertLock(bool aAssert) noexcept;
        static bool AssertLock() noexcept;

        static void TimeStamp(bool aTimeStamp) noexcept;
        static bool TimeStamp() noexcept;

        static void TimeStampFractionSeconds(std::streamsize aWidth) noexcept;
        static std::streamsize TimeStampFractionSeconds() noexcept;
    private:
        struct Instance;
        std::shared_ptr<Instance> _instance;
    };
}

#define MOO_LOG_FUNCTION \
{ \
    moo::Logger::Lock lock; \
    moo::clog_noExcept << __FUNCTION__ << std::endl; \
}

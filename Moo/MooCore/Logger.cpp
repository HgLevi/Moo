#include "Logger.h"

#include "RedirectStream.hpp"
#include "Time.hpp"

#include <mutex>
#include <fstream>
#include <set>

#include <windows.h>

using namespace std;
using namespace moo;

namespace {
    constexpr bool cAssertSingleInstance = false;
    constexpr bool cTimeStampDate = false;

    class Flusher {
    public:
        Flusher(ostream* pOS) noexcept;
        void FlushLastIfNeeded();
    private:
        ostream* _pOS;
    };

    class Prefixer {
    public:
        Prefixer(string aPrefix) noexcept;
        void AddPrefix(string& aStr);
    protected:
        size_t PrefixSize() const noexcept;
        string Prefix() const;

        string _prefix;
        bool _lastCharWasNewLine = true;
    };

    class PreWriter : private Flusher, private Prefixer {
    public:
        PreWriter(ostream* pOS, string aPrefix = "") noexcept;
        string PreWrite(string aStr);
    };

    class DebugLogger : private PreWriter {
    public:
        DebugLogger(ostream* pOS, string aPrefix = "") noexcept;
        void operator()(string aStr);
    };

    class FileLogger : private PreWriter {
    public:
        FileLogger(ostream* pOS, ofstream& aFile, string aPrefix = "");
        void operator()(string aStr);
    private:
        ofstream& _file;
    };

    class DebugAndFileLogger {
    public:
        DebugAndFileLogger(ostream* pOS, ofstream& aFile, string aPrefix = "");
        void operator()(string aStr);
    private:
        DebugLogger _debugLogger;
        FileLogger _fileLogger;
    };

    bool s_timeStamp = true;
    streamsize s_fractionSeconds = 4;

    size_t s_lockCount = 0;
    bool s_assertLock = true;

    void AssertLock() noexcept;
}

//----------------------------------------------------------------------------------------------------------------------

struct Logger::Instance {
    Instance(const string& aLogPath);

    ofstream _file;

    RedirectStream<DebugLogger> _coutRedirectStream;
    RedirectStream<DebugAndFileLogger> _clogRedirectStream;
    RedirectStream<DebugAndFileLogger> _cerrRedirectStream;

    static inline weak_ptr<Instance> s_instance;
    static inline mutex s_instanceMutex;

    static inline string s_defaultLogPath = "Moo.log";

    static inline recursive_mutex s_logMutex;

    static inline set<string> s_openedLogPaths;

    template<class T, class... Args>
    static RedirectStream<T> CreateRedirect(ostream& aOS, Args&&... aArgs);

    static ios::_Openmode Openmode(const string& aLogPath);
};

Logger::Instance::Instance(const string& aLogPath)
    : _file(aLogPath, Openmode(aLogPath))
    , _coutRedirectStream(CreateRedirect<DebugLogger>(cout, " out | "))
    , _clogRedirectStream(CreateRedirect<DebugAndFileLogger>(clog, _file, " log | "))
    , _cerrRedirectStream(CreateRedirect<DebugAndFileLogger>(cerr, _file, "-ERR-| "))
{
}

Logger::Logger(const string& aLogPath) noexcept
{
    NoExcept([&]()
        {
            scoped_lock lock(Instance::s_instanceMutex);

            if constexpr (cAssertSingleInstance)
            {
                MOO_ASSERT(Instance::s_instance.expired());
            }

            if (Instance::s_instance.expired())
            {
                _instance = make_shared<Instance>(aLogPath != "" ? aLogPath : Instance::s_defaultLogPath);
                Instance::s_instance = _instance;

                Lock logLock;
                clog << "moo::Logger started" << endl;
            }
        },
        MOO_WHERE);
}

Logger::~Logger()
{
    NoExcept([&]()
        {
            scoped_lock lock(Instance::s_instanceMutex);

            if (_instance.use_count() == 1)
            {
                Lock logLock;
                clog << "moo::Logger shutting down" << endl;
                // Warning:
                // If this line is removed, then maybe a flush will be needed in case something is left in a stream
            }

            _instance.reset();
        },
        MOO_WHERE);
}

template<class T, class... Args>
RedirectStream<T> Logger::Instance::CreateRedirect(ostream& aOS, Args&&... aArgs)
{
    return RedirectStream<T>(&aOS, &aOS, forward<Args>(aArgs)...);
}

ios::_Openmode Logger::Instance::Openmode(const string& aLogPath)
{
    const bool inserted = s_openedLogPaths.insert(aLogPath).second;
    return inserted ? ios::trunc : ios::app;
}

MOO_SUPPRESS(26115) // Failing to release lock
Logger::Lock::Lock() noexcept
    : _lock(NoExcept([&]() { return unique_lock(Instance::s_logMutex); }, MOO_WHERE))
{
    if (_lock.owns_lock())
    {
        ++s_lockCount;
    }
}

Logger::Lock::~Lock()
{
    if (_lock.owns_lock())
    {
        --s_lockCount;
    }
}

//----------------------------------------------------------------------------------------------------------------------

Flusher::Flusher(ostream* pOS) noexcept
    : _pOS(pOS)
{
}

void Flusher::FlushLastIfNeeded()
{
    static ostream* s_pLastOS = nullptr;
    static bool s_flushingLast = false;

    if (s_flushingLast)
    {
        return;
    }

    if (s_pLastOS && s_pLastOS != _pOS)
    {
        s_flushingLast = true;
        s_pLastOS->flush();
        s_flushingLast = false;
    }

    s_pLastOS = _pOS;
}

//----------------------------------------------------------------------------------------------------------------------

Prefixer::Prefixer(string aPrefix) noexcept
    : _prefix(move(aPrefix))
{
}

size_t Prefixer::PrefixSize() const noexcept
{
    if (!s_timeStamp)
    {
        return _prefix.size();
    }

    return _prefix.size() + TimeStampSize(s_fractionSeconds, cTimeStampDate) + s_fractionSeconds;
}

string Prefixer::Prefix() const
{
    if (!s_timeStamp)
    {
        return _prefix;
    }

    return TimeStampString(s_fractionSeconds, cTimeStampDate) + _prefix;
}

void Prefixer::AddPrefix(string& aStr)
{
    if (aStr.size() == 0)
    {
        return;
    }

    if (_lastCharWasNewLine)
    {
        aStr.insert(0, Prefix());
    }

    for (size_t i = 0; i < aStr.size(); i += PrefixSize() + 1)
    {
        i = aStr.find("\n", i);

        if (i >= aStr.size())
        {
            break;
        }

        if (i == aStr.size() - 1)
        {
            _lastCharWasNewLine = true;
            return;
        }

        aStr.insert(i + 1, Prefix());
    }

    _lastCharWasNewLine = false;
}

//----------------------------------------------------------------------------------------------------------------------

PreWriter::PreWriter(ostream* pOS, string aPrefix) noexcept
    : Flusher(pOS)
    , Prefixer(move(aPrefix))
{
}

string PreWriter::PreWrite(string aStr)
{
    AssertLock();
    FlushLastIfNeeded();
    AddPrefix(aStr);
    return aStr;
}

//----------------------------------------------------------------------------------------------------------------------

DebugLogger::DebugLogger(ostream* pOS, string aPrefix) noexcept
    : PreWriter(pOS, move(aPrefix))
{
}

void DebugLogger::operator()(string aStr)
{
    aStr = PreWrite(aStr);
    OutputDebugString(aStr.c_str());
}

//----------------------------------------------------------------------------------------------------------------------

FileLogger::FileLogger(ostream* pOS, ofstream& aFile, string aPrefix)
    : PreWriter(pOS, move(aPrefix))
    , _file(aFile)
{
}

void FileLogger::operator()(string aStr)
{
    aStr = PreWrite(aStr);
    _file << aStr.c_str();
}

//----------------------------------------------------------------------------------------------------------------------

DebugAndFileLogger::DebugAndFileLogger(ostream* pOS, ofstream& aFile, string aPrefix)
    : _debugLogger(pOS, aPrefix)
    , _fileLogger(pOS, aFile, move(aPrefix)) {}

void DebugAndFileLogger::operator()(string aStr)
{
    _debugLogger(aStr);
    _fileLogger(move(aStr));
}

//----------------------------------------------------------------------------------------------------------------------

void ::AssertLock() noexcept
{
    if (s_assertLock)
    {
        MOO_ASSERT(s_lockCount > 0);
    }
}

//----------------------------------------------------------------------------------------------------------------------

//static
void Logger::DefaultLogPath(string aLogPath) noexcept
{
    Instance::s_defaultLogPath = move(aLogPath);
}
//static
const string& Logger::DefaultLogPath() noexcept
{
    return Instance::s_defaultLogPath;
}

//static
void Logger::AssertLock(bool aAssert) noexcept
{
    s_assertLock = aAssert;
}
//static
bool Logger::AssertLock() noexcept
{
    return s_assertLock;
}

//static
void Logger::TimeStamp(bool aTimeStamp) noexcept
{
    s_timeStamp = aTimeStamp;
}
//static
bool Logger::TimeStamp() noexcept
{
    return s_timeStamp;
}

//static
void Logger::TimeStampFractionSeconds(streamsize aWidth) noexcept
{
    s_fractionSeconds = aWidth;
}
//static
streamsize Logger::TimeStampFractionSeconds() noexcept
{
    return s_fractionSeconds;
}

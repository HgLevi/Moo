#pragma once

#include "ScopedArray.hpp"
#include "MooDefaults.h"
#include "NoExcept.hpp"

#include <sstream>

#include <functional>

namespace moo {
    template<class T>
    concept RedirectStreamTarget = std::invocable<T, std::string>;

    template<class T>
    concept NotRedirectStreamTarget = !RedirectStreamTarget<T>;

    template<RedirectStreamTarget T>
    constexpr std::function<void(std::string)> ToFunction(T&& aRedirectStreamTarget) noexcept
    {
        using namespace std;
        return function<void(string)>(forward<T>(aRedirectStreamTarget));
    }

    using StreamList = std::initializer_list<std::ostream*>;

    template<RedirectStreamTarget T>
    class RedirectStream : private std::stringbuf, public std::ostream {
    public:
        RedirectStream(StreamList aOSPtrs, T&& aTarget);

        RedirectStream(std::ostream* apOS, T&& aTarget)
            : RedirectStream({ apOS }, std::move(aTarget)) {}

        template<NotRedirectStreamTarget... Args>
        RedirectStream(StreamList aOSPtrs, Args&&... aArgs)
            : RedirectStream(aOSPtrs, std::move(T(std::forward<Args>(aArgs)...))) {}

        template<NotRedirectStreamTarget... Args>
        RedirectStream(std::ostream* apOS, Args&&... aArgs)
            : RedirectStream({apOS}, std::forward<Args>(aArgs)...) {}

        MOO_DELETE_DEFAULTS(RedirectStream);

    private:
        int sync() noexcept override;

        struct StreamPtrs;

        T _target;
        ScopedArray<StreamPtrs> _streamPtrs;
    };
}

template<moo::RedirectStreamTarget T>
moo::RedirectStream<T>::RedirectStream(StreamList aOSPtrs, T&& aTarget)
    : std::ostream(this)
    , _target(std::move(aTarget))
    , _streamPtrs(aOSPtrs)
{
    for (std::ostream* pOs : aOSPtrs)
    {
        MOO_ASSERT_NOT_NULL(pOs);

        pOs->rdbuf(this);
        pOs->tie(this);
    }
}

template<moo::RedirectStreamTarget T>
int moo::RedirectStream<T>::sync() noexcept
{
    return NoExceptSuccess([&]()
        {
            std::string myStr = str();
            if (myStr != "")
            {
                _target(myStr);
                str("");
            }
        }
        , MOO_WHERE) ? 0 : -1;
}

//----------------------------------------------------------------------------------------------------------------------

template<moo::RedirectStreamTarget T>
struct moo::RedirectStream<T>::StreamPtrs {
    StreamPtrs() = default;
    StreamPtrs(std::ostream* apOS) noexcept;
    ~StreamPtrs();
    StreamPtrs(StreamPtrs&& aOther) noexcept;
    StreamPtrs& operator=(StreamPtrs&& aOther) noexcept;
    StreamPtrs(const StreamPtrs& aOther) = delete;
    StreamPtrs& operator=(const StreamPtrs& aOther) = delete;

    std::ostream* _pOS = nullptr;
    std::streambuf* _pOldBuf = nullptr;
    std::ostream* _pOldTie = nullptr;
};

template<moo::RedirectStreamTarget T>
moo::RedirectStream<T>::StreamPtrs::StreamPtrs(std::ostream* apOS) noexcept
    : _pOS(apOS)
    , _pOldBuf(apOS->rdbuf())
    , _pOldTie(apOS->tie())
{
    MOO_ASSERT_NOT_NULL(apOS);
}

template<moo::RedirectStreamTarget T>
moo::RedirectStream<T>::StreamPtrs::~StreamPtrs()
{
    if (_pOS)
    {
        NoExcept([&]()
            {
                _pOS->rdbuf(_pOldBuf);
                _pOS->tie(_pOldTie);
            }, MOO_WHERE);
    }
}

template<moo::RedirectStreamTarget T>
moo::RedirectStream<T>::StreamPtrs::StreamPtrs(StreamPtrs&& aOther) noexcept
{
    *this = move(aOther);
}

template<moo::RedirectStreamTarget T>
moo::RedirectStream<T>::StreamPtrs&
    moo::RedirectStream<T>::StreamPtrs::operator=(StreamPtrs&& aOther) noexcept
{
    _pOS = aOther._pOS;
    _pOldBuf = aOther._pOldBuf;
    _pOldTie = aOther._pOldTie;

    aOther._pOS = nullptr;
    aOther._pOldBuf = nullptr;
    aOther._pOldTie = nullptr;
    return *this;
}

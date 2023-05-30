#pragma once

#define MOO_EXCLUDE_IOS_INCLUDES

#include "Concepts.h"
#include "MooDefaults.h"
#include "MooWarning.h"

#include <memory>
#include <span>

namespace moo {
    template<class T>
    struct ScopedArray {
        using Type = T;
        using Iterator = std::_Span_iterator<T>;
        using ConstIterator = std::_Span_iterator<const T>;

        std::unique_ptr<T[]> _data;
        size_t _size = 0;

        ScopedArray() = default;
        ~ScopedArray() = default;
        explicit constexpr ScopedArray(size_t aSize);
        template<class Range> requires ConstructibleFromRangeElement<T, Range>
        explicit constexpr ScopedArray(Range aRange);
        template<class Range> requires ConstructibleFromRangeElement<T, Range>
        ScopedArray& operator=(Range aRange);
        MOO_NO_COPY(ScopedArray);

        [[nodiscard]] constexpr Iterator begin() noexcept { return begin<T>(this); }
        [[nodiscard]] constexpr ConstIterator begin() const noexcept { return begin<const T>(this); }

        [[nodiscard]] constexpr Iterator end() noexcept { return end<T>(this); }
        [[nodiscard]] constexpr ConstIterator end() const noexcept { return end<const T>(this); }

        [[nodiscard]] constexpr T& operator[](size_t aIndex) noexcept;
        [[nodiscard]] constexpr const T& operator[](size_t aIndex) const noexcept;

        [[nodiscard]] constexpr size_t size() const noexcept;

        [[nodiscard]] constexpr T* data() noexcept;
        [[nodiscard]] constexpr const T* data() const noexcept;

        constexpr void reset() noexcept;
        constexpr void Reset(size_t aSize = 0);
        // Warning! Always make sure apData was allocated using 'new[]' like so:
        //   apData = new T[aSize]
        constexpr void Reset(T* apData, size_t aSize = 0) noexcept;

        [[nodiscard]] constexpr std::span<const char> CBytes() const noexcept;
        [[nodiscard]] constexpr std::span<const unsigned char> CUBytes() const noexcept;

    private:

        template<class Tv, class TSelf>
        [[nodiscard]] static inline constexpr std::_Span_iterator<Tv> begin(TSelf* apSelf) noexcept;
        template<class Tv, class TSelf>
        [[nodiscard]] static inline constexpr std::_Span_iterator<Tv> end(TSelf* apSelf) noexcept;
    };
}

template<class T>
constexpr moo::ScopedArray<T>::ScopedArray(size_t aSize)
    : _data(std::make_unique<T[]>(aSize))
    , _size(aSize)
{}

template<class T>
template<class Range>
    requires moo::ConstructibleFromRangeElement<T, Range>
constexpr moo::ScopedArray<T>::ScopedArray(Range aRange)
{
    *this = aRange;
}

template<class T>
template<class Range>
    requires moo::ConstructibleFromRangeElement<T, Range>
moo::ScopedArray<T>& moo::ScopedArray<T>::operator=(Range aRange)
{
    Reset(aRange.size());

    auto itData = begin();
    for (auto& item : aRange)
    {
        *itData = T(std::move(item));
        ++itData;
    }

    return *this;
}

template<class T>
[[nodiscard]] constexpr T& moo::ScopedArray<T>::operator[](size_t aIndex) noexcept
{
    MOO_RETURN_RANDOM_ACCESS(data(), aIndex, _size);
}
template<class T>
[[nodiscard]] constexpr const T& moo::ScopedArray<T>::operator[](size_t aIndex) const noexcept
{
    MOO_RETURN_RANDOM_ACCESS(data(), aIndex, _size);
}

template<class T>
[[nodiscard]] constexpr size_t moo::ScopedArray<T>::size() const noexcept
{
    return _size;
}

template<class T>
[[nodiscard]] constexpr T* moo::ScopedArray<T>::data() noexcept
{
    return _data.get();
}
template<class T>
[[nodiscard]] constexpr const T* moo::ScopedArray<T>::data() const noexcept
{
    return _data.get();
}

template<class T>
constexpr void moo::ScopedArray<T>::reset() noexcept
{
    Reset();
}
template<class T>
constexpr void moo::ScopedArray<T>::Reset(size_t aSize)
{
    _size = aSize;

    if (_size == 0)
    {
        _data.reset();
        return;
    }

    _data = std::make_unique<T[]>(_size);
}
template<class T>
constexpr void moo::ScopedArray<T>::Reset(T* apData, size_t aSize) noexcept
{
    MOO_ASSERT_RETURN(!apData == !aSize);

    _data.reset(apData);
    _size = aSize;
}

template<class T>
[[nodiscard]] constexpr std::span<const char> moo::ScopedArray<T>::CBytes() const noexcept
{
    return ToCBytes(*this);
}
template<class T>
[[nodiscard]] constexpr std::span<const unsigned char> moo::ScopedArray<T>::CUBytes() const noexcept
{
    return ToCUBytes(*this);
}

//-------------------------------------------------------------------------------------------------------
// private:

template<class T>
template<class Tv, class TSelf>
[[nodiscard]] static inline constexpr std::_Span_iterator<Tv> moo::ScopedArray<T>::begin(TSelf* apSelf) noexcept
{
    Tv* ptr = apSelf->data();
#if MOO_ITERATOR_DEBUG_LEVEL >= 1
    MOO_SUPPRESS(26481); // Don't use pointer arithmetic. Use span instead
    return { ptr, ptr, ptr + apSelf->_size };
#else
    return { ptr };
#endif
}
template<class T>
template<class Tv, class TSelf>
[[nodiscard]] static inline constexpr std::_Span_iterator<Tv> moo::ScopedArray<T>::end(TSelf* apSelf) noexcept
{
    Tv* ptr = apSelf->data();
    MOO_SUPPRESS(26481); // Don't use pointer arithmetic. Use span instead
    Tv* end = ptr + apSelf->_size;
#if MOO_ITERATOR_DEBUG_LEVEL >= 1
    return { end, ptr, end };
#else
    return { end };
#endif
}

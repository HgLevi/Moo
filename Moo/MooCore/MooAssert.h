#pragma once
#include <corecrt.h>

_CRT_BEGIN_C_HEADER

#ifdef NDEBUG

#define MOO_ASSERT(expression) ((void)(expression))

#else

_ACRTIMP void __cdecl _wassert(
    _In_z_ wchar_t const* _Message,
    _In_z_ wchar_t const* _File,
    _In_   unsigned       _Line
);

#define MOO_ASSERT(expression) (                                                                 \
            (!!(expression)) ||                                                              \
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
        )

#endif

_CRT_END_C_HEADER

//----------------------------------------------------------------------------------------------------------------------

#define MOO_ASSERT_RETURN(arg, ...) \
    MOO_ASSERT(!!(arg)); \
    if (!(arg)) \
    { \
        return __VA_ARGS__; \
    }

#ifdef NDEBUG
#define MOO_ASSERT_NOT_NULL(arg, ...) MOO_ASSERT((arg))
#else
#define MOO_ASSERT_NOT_NULL(arg, ...) MOO_ASSERT_RETURN((arg), __VA_ARGS__)
#endif // NDEBUG

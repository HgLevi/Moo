#pragma once

#define MOO_UNUSED(x) (void)(x)

#define MOO_STRINGIFY(x) #x

#define MOO_SUPPRESS(x) _Pragma(MOO_STRINGIFY(warning(suppress:##x)))
#define MOO_DISABLE(x) _Pragma(MOO_STRINGIFY(warning(disable:##x)))
#define MOO_WARNING_PUSH _Pragma("warning(push)")
#define MOO_WARNING_POP _Pragma("warning(pop)")

#define MOO_ITERATOR_DEBUG_LEVEL _ITERATOR_DEBUG_LEVEL
#define MOO_CONTAINER_DEBUG_LEVEL _CONTAINER_DEBUG_LEVEL

// 26477: Use 'nullptr' rather than 0 or NULL
#define MOO_VERIFY(A_COND, A_MSG) MOO_SUPPRESS(26477) _STL_VERIFY(A_COND, A_MSG)

#if MOO_CONTAINER_DEBUG_LEVEL > 0
    #define MOO_VERIFY_INDEX(A_INDEX, A_SIZE) MOO_VERIFY(A_INDEX < A_SIZE, "index out of range")
#else
    #define MOO_VERIFY_INDEX(A_INDEX, A_SIZE)
#endif

// 26481: Don't use pointer arithmetic. Use span instead
#define MOO_RETURN_RANDOM_ACCESS(A_BEGIN, A_INDEX, A_SIZE) \
MOO_VERIFY_INDEX(A_INDEX, A_SIZE); \
MOO_SUPPRESS(26481) \
return A_BEGIN[A_INDEX]

#pragma once

#include <concepts>

namespace moo {
    template<class T, class Range>
    concept ConstructibleFromRangeElement = requires(Range aRange)
    {
        { T(*aRange.begin()) };
    };
}

#define MOO_RETURN_TYPE(A_FUNCTYPE, A_FUNC, A_ARGSTYPE, A_ARGS)\
    decltype(std::invoke(std::forward<A_FUNCTYPE>(A_FUNC), std::forward<A_ARGSTYPE>(A_ARGS)...))

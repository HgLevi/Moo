#pragma once
#include <concepts>

namespace moo {
    template<std::integral T>
    constexpr T Pow(T aBase, T aExp) noexcept
    {
        T result = 1;

        while (aExp > 0)
        {
            if (aExp & 1)
            {
                result *= aBase;
            }
            aExp /= 2;
            aBase *= aBase;
        }

        return result;
    }
}

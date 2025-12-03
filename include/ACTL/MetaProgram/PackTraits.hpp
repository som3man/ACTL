#pragma once

#include "Constant.hpp"

namespace ACTL {
    namespace Internal {
        template <size currentSize, typename CurrentType, typename... OtherTypes>
        struct GetBiggestPackSize {
            static constexpr size nextSize = currentSize > sizeof(CurrentType) ? currentSize : sizeof(CurrentType);

            static constexpr size result = GetBiggestPackSize<nextSize, OtherTypes...>::result;
        };

        template <size currentSize, typename CurrentType>
        struct GetBiggestPackSize<currentSize, CurrentType> {
            static constexpr size result = currentSize > sizeof(CurrentType) ? currentSize : sizeof(CurrentType);
        };

        template <size currentIndex, typename TargetType, typename CurrentType, typename... OtherTypes>
        struct GetPackTypeIndex {
            static constexpr size result = IsSame<TargetType, CurrentType>() ? currentIndex : GetPackTypeIndex<currentIndex + 1, TargetType, OtherTypes...>::result;
        };

        template <size currentIndex, typename TargetType, typename CurrentType>
        struct GetPackTypeIndex<currentIndex, TargetType, CurrentType> {
            static constexpr size result = currentIndex;
        };

        template <size targetIndex, size currentIndex, typename CurrentType, typename... OtherTypes>
        struct GetPackTypeByIndex {
            using Result = Conditional<targetIndex == currentIndex, CurrentType, typename GetPackTypeByIndex<targetIndex, currentIndex + 1, OtherTypes...>::Result>;
        };

        template <size targetIndex, size currentIndex, typename CurrentType>
        struct GetPackTypeByIndex<targetIndex, currentIndex, CurrentType> {
            using Result = CurrentType;
        };

        template <size prevSize, typename CurrentType, typename... OtherTypes>
        struct GetPackSize {
            static constexpr size thisSize = sizeof(CurrentType);

            static constexpr size currentSize = thisSize + prevSize;

            static constexpr size result = GetPackSize<currentSize, OtherTypes...>::result;
        };

        template <size prevSize, typename CurrentType>
        struct GetPackSize<prevSize, CurrentType> {
            static constexpr size thisSize = sizeof(CurrentType);

            static constexpr size currentSize = thisSize + prevSize;

            static constexpr size result = currentSize;
        };
    }

    // Size of the biggest type in a pack.
    template <typename... Pack>
    static constexpr size biggestPackSize = Internal::GetBiggestPackSize<0, Pack...>::result;

    // Index of given type in a pack.
    template <typename Target, typename... Pack>
    static constexpr size packTypeIndex = Internal::GetPackTypeIndex<0, Target, Pack...>::result;

    // Type from a pack located by index.
    template <size index, typename... Pack>
    using PackTypeByIndex = typename Internal::GetPackTypeByIndex<index, 0, Pack...>::Result;

    // First type from a pack.
    template <typename... Pack>
    using FirstPackType = PackTypeByIndex<0, Pack...>;

    // Sum of each pack element memory sizes
    template <typename... Pack>
    static constexpr size fullPackSize = Internal::GetPackSize<0, Pack...>::result;
}
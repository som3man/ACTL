module;

#include <type_traits>

export module ACTL:Traits;

import :Defines;

namespace ACTL {
    export template <typename TypeA, typename TypeB>
    constexpr bool areSame = std::is_same_v<TypeA, TypeB>;

    export template <typename Type>
    constexpr bool isConst = std::is_const_v<Type>;

    export template <typename Type>
    constexpr bool isReference = std::is_reference_v<Type>;

    export template <typename Type>
    constexpr bool isFunction = std::is_function_v<Type>;

    export template <typename Return, typename Func, typename... Args>
    constexpr bool isInvocable = std::is_invocable_r_v<Return, std::remove_reference_t<Func>, Args...>;

    export template <typename Return, typename Func, typename... Args>
    constexpr bool isNoexceptInvocable = std::is_nothrow_invocable_r_v<Return, std::remove_reference_t<Func>, Args...>;

    export template <typename Base, typename Derived>
    constexpr bool isBaseOf = std::is_base_of_v<Base, Derived>;

    export template <typename Type, typename... Args>
    constexpr bool hasConstructor = std::is_constructible_v<std::remove_reference_t<Type>, Args...>;

    export template <typename Type, typename Other>
    constexpr bool hasAssignment = std::is_assignable_v<std::remove_reference_t<Type>&, Other>;

    export template <typename Type, typename... Args>
    constexpr bool hasNoexceptConstructor = std::is_nothrow_constructible_v<std::remove_reference_t<Type>, Args...>;

    export template <typename Type, typename Other>
    constexpr bool hasNoexceptAssignment = std::is_nothrow_assignable_v<std::remove_reference_t<Type>&, Other>;

    export template <typename Type>
    constexpr bool hasNoexceptDestructor = std::is_nothrow_destructible_v<std::remove_reference_t<Type>>;

    export template <typename Type>
    constexpr bool hasVirtualDestructor = std::has_virtual_destructor_v<std::remove_reference_t<Type>>;

    export template <typename Type>
    constexpr bool hasTrivialDestructor = std::is_trivially_destructible_v<std::remove_reference_t<Type>>;
}
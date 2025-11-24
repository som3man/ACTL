#pragma once

#include "Constant.hpp"

namespace ACTL {
	// Checks if type is a reference.
	template <typename Type>
	struct IsReference : False {};

	// Checks if type is a reference.
	template <typename Type>
	struct IsReference<Type&> : True {};

	// Checks if type is a reference.
	template <typename Type>
	struct IsReference<Type&&> : True {};

	// Checks if type is a reference.
	template <typename Type>
	constexpr bool isReference = IsReference<Type>::value;

	// Checks if type is a lvalue reference.
	template <typename Type>
	struct IsLvalueReference : False {};

	// Checks if type is a lvalue reference.
	template <typename Type>
	struct IsLvalueReference<Type&> : True {};

	// Checks if type is a lvalue reference.
	template <typename Type>
	constexpr bool isLvalueReference = IsLvalueReference<Type>::value;

	namespace Internal {
		template <typename TemplateType, bool isRef>
		struct RvalueReference {
			using Type = TemplateType&&;
		};

		template <typename TemplateType>
		struct RvalueReference<TemplateType, true> {
			using Type = TemplateType;
		};
	}

	template <typename TemplateType>
	struct RvalueReference {
		using Type = Internal::RvalueReference<TemplateType, isReference<TemplateType>>::Type;
	};

	template <typename Type>
	using RvalueReferenceType = typename RvalueReference<Type>::Type;

	template <typename Type>
	typename RvalueReferenceType<Type> declval() noexcept;

	namespace Internal {
		template <typename TemplateType, typename... Args>
		struct HasConstructor {
			template <typename Type2, typename... Args2>
			static auto Test(int) -> decltype(void(Type2(ACTL::declval<Args2>()...)), True());

			template <typename, typename...>
			static auto Test(...) -> False;

			using Type = decltype(Test<TemplateType, Args...>(0));
		};

		template <typename Type>
		struct HasCopyConstructor : HasConstructor<Type, const Type&> {};

		template <typename Type>
		struct HasMoveConstructor : HasConstructor<Type, RvalueReferenceType<Type>> {};

		template <typename Type, typename... Args>
		auto ConstructionTest(int) -> decltype(Type(ACTL::declval<Args>()...), ACTL::True());

		template <typename Type, typename... Args>
		auto ConstructionTest(int) -> ACTL::False;
	}

	// Checks if type has a contructor with given arguments.
	template <typename Type, typename... Args>
	constexpr bool hasConstructor = Internal::HasConstructor<Type, Args...>::Type::value;

	// Checks if type has a copy constructor.
	template <typename Type>
	constexpr bool hasCopyConstructor = Internal::HasCopyConstructor<Type>::Type::value;

	// Checks if type has a move constructor.
	template <typename Type>
	constexpr bool hasMoveConstructor = Internal::HasMoveConstructor<Type>::Type::value;
}
#pragma once

#include "../Defines.hpp"

namespace ACTL {
	// Compile time constant with info.
	template <typename TemplateType, TemplateType templateValue>
	struct Constant {
		static constexpr TemplateType value = templateValue;

		using Type = TemplateType;

		using This = Constant<Type, value>;

		constexpr operator Type() const noexcept {
			return value;
		}

		constexpr Type operator()() const noexcept {
			return value;
		}
	};

	template <bool value>
	using Bool = Constant<bool, value>;

	using False = Bool<false>;

	using True = Bool<true>;

	template <typename First, typename Second>
	struct IsSame : False {};

	template <typename Type>
	struct IsSame<Type, Type> : True {};

	namespace Internal {
		template <bool condition, typename TrueType, typename FalseType>
		struct Conditional {
			using Result = TrueType;
		};

		template <typename TrueType, typename FalseType>
		struct Conditional<false, TrueType, FalseType> {
			using Result = FalseType;
		};
	}

	template <bool condition, typename TrueType, typename FalseType>
	using Conditional = typename Internal::Conditional<condition, TrueType, FalseType>::Result;
}

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
}
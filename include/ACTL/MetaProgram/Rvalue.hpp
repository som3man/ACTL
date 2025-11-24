#pragma once

#include "TypeTraits.hpp"

namespace ACTL {
	// Returns type without ampersand(-s).
	template <typename TemplateType>
	struct NoReference {
		using Type = TemplateType;
	};

	// Returns type without ampersand(-s).
	template <typename TemplateType>
	struct NoReference<TemplateType&> {
		using Type = TemplateType;
	};

	// Returns type without ampersand(-s).
	template <typename TemplateType>
	struct NoReference<TemplateType&&> {
		using Type = TemplateType;
	};

	// Returns type without ampersand(-s).
	template <typename Type>
	using NoReferenceType = typename NoReference<Type>::Type;

	template <typename Type>
	constexpr Type&& forward(NoReferenceType<Type>& value) noexcept {
		return static_cast<Type&&>(value);
	}

	template <typename Type>
	constexpr Type&& forward(NoReferenceType<Type>&& value) noexcept {
		static_assert(!isLvalueReference<Type>, "Cannot forward a rvalue as a lvalue.");

		return static_cast<Type&&>(value);
	}

	template <typename Type>
	constexpr NoReferenceType<Type>&& move(Type&& value) noexcept {
		return static_cast<NoReferenceType<Type>&&>(value);
	}

	template <typename Type>
	constexpr void swap(Type& left, Type& right) noexcept {
		Type temp = ACTL::move(left);

		left = ACTL::move(right);

		right = ACTL::move(temp);
	}
}
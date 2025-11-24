#pragma once

#include "../Defines.hpp"

namespace ACTL {
	// Simple struct for c-style arrays.
	template <typename Type>
	struct Many {
	public:
		// Pointer to an array.
		Type* data = nullptr;

		// Length of array.
		size count = 0;

		constexpr Many() noexcept = default;

		constexpr Many(Type* data, size count) noexcept : data(data), count(count) {};

		constexpr ~Many() noexcept {
			data = nullptr;

			count = 0;
		}

		constexpr Type& operator[](size index) noexcept {
			return data[index];
		};

		constexpr const Type& operator[](size index) const noexcept {
			return data[index];
		};

		constexpr Type* begin() noexcept {
			return data;
		};

		constexpr const Type* begin() const noexcept {
			return data;
		};

		constexpr Type* end() noexcept {
			return data + count;
		};

		constexpr const Type* end() const noexcept {
			return data + count;
		};
	};
}
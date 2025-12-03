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

		Many() noexcept = default;

		Many(Type* data, size count) noexcept : data(data), count(count) {};

		~Many() noexcept {
			data = nullptr;

			count = 0;
		}

		Type& operator[](size index) noexcept {
			return data[index];
		};

		const Type& operator[](size index) const noexcept {
			return data[index];
		};

		Type* begin() noexcept {
			return data;
		};

		const Type* begin() const noexcept {
			return data;
		};

		Type* end() noexcept {
			return data + count;
		};

		const Type* end() const noexcept {
			return data + count;
		};
	};
}

/*
MIT License

Copyright (c) 2026 som3man

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

module;

#include <cstdint>
#include <utility>

export module ACTL:Defines;

export namespace ACTL {
    // 1 byte Signed.
    using i8 = std::int8_t;

    // 1 byte UNsigned.
    using u8 = std::uint8_t;

    // 2 byte Signed.
    using i16 = std::int16_t;

    // 2 byte UNsigned.
    using u16 = std::uint16_t;

    // 4 byte Signed.
    using i32 = std::int32_t;

    // 4 byte UNsigned.
    using u32 = std::uint32_t;

    // 8 byte Signed.
    using i64 = std::int64_t;

    // 8 byte UNsigned.
    using u64 = std::uint64_t;

    // 4 byte floating point.
    using f32 = float;

    // 8 byte floating point.
    using f64 = double;

    [[nodiscard]] constexpr auto&& Move(auto&& data) noexcept {
        return std::move(data);
    }

    template <typename Type>
    [[nodiscard]] constexpr Type&& Forward(Type& data) noexcept {
        return std::forward<Type>(data);
    }

    template <class Type>
	concept Swapable = requires (Type object, Type& ref) {
		{ object.Swap(ref) };
	};

	template <Swapable Type>
	constexpr void Swap(Type& left, Type& right) noexcept {
		left.Swap(right);
	}

	template <typename Type>
	constexpr void Swap(Type& left, Type& right) noexcept {
		std::swap(left, right);
	}
}
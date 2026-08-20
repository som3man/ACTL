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
#include <cstdlib>

export module ACTL:Defines;

export namespace ACTL {
    using u64 = uint64_t;

	using i64 = int64_t;

	using u32 = uint32_t;

	using i32 = int32_t;

	using u16 = uint16_t;

	using i16 = int16_t;

	using u8 = uint8_t;

	using i8 = int8_t;

	using f32 = float;

	using f64 = double;

	using size = size_t;

	using byte = u8;

	constexpr size sizemax = SIZE_MAX;

	constexpr u64 u64max = UINT64_MAX;

	constexpr i64 i64max = INT64_MAX;

	constexpr i64 i64min = INT64_MIN;

	constexpr u32 u32max = UINT32_MAX;

	constexpr i32 i32max = INT32_MAX;

	constexpr i32 i32min = INT32_MIN;

	constexpr u16 u16max = UINT16_MAX;

	constexpr i16 i16max = INT16_MAX;

	constexpr i16 i16min = INT16_MIN;

	constexpr u8 u8max = UINT8_MAX;

	constexpr i8 i8max = INT8_MAX;

	constexpr i8 i8min = INT8_MIN;

	constexpr f32 f32max = 3.402823466e+38F;

	constexpr f32 f32min = 1.175494351e-38F;

	constexpr f64 f64max = 1.7976931348623158e+308;

	constexpr f64 f64min = 2.2250738585072014e-308;
}
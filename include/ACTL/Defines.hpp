#pragma once

#include <assert.h>
#include <math.h>

#if defined(_DEBUG) || !defined(NDEBUG)
	#define ACTL_DEBUG
#endif

namespace ACTL {
	constexpr bool debug =
#ifdef ACTL_DEBUG
		true;
#else
		false;
#endif
}

#ifndef ACTL_DEBUG
	#define assert(X) ((void)0)
#endif

#if defined(_VECTOR_)
	#define ACTL_STD_INCLUDED_VECTOR 
#endif

#if defined(_XSTRING_)
	#define ACTL_STD_INCLUDED_STRING 
#endif

#if defined(_IOSTREAM_)
	#define ACTL_STD_INCLUDED_IOSTREAM 
#endif

namespace ACTL {
	using u64 = unsigned long long;

	using i64 = long long;

	using u32 = unsigned long;

	using i32 = long;

	using u16 = unsigned short;

	using i16 = short;

	using u8 = unsigned char;

	using i8 = char;

	using f32 = float;

	using f64 = double;

	using size = u64;

	template <typename Char>
	using cstr = Char*;

	constexpr u64 u64max = 0xFFFFFFFFFFFFFFFF;

	constexpr i64 i64max = 0x7FFFFFFFFFFFFFFF;

	constexpr i64 i64min = 0x8000000000000000;

	constexpr u32 u32max = 0xFFFFFFFF;

	constexpr i32 i32max = 0x7FFFFFFF;

	constexpr i32 i32min = 0x80000000;

	constexpr u16 u16max = 0xFFFF;

	constexpr i16 i16max = 32767;

	constexpr i16 i16min = -32768;

	constexpr u8 u8max = 0xFF;

	constexpr i8 i8max = 127;

	constexpr i8 i8min = -128;

	constexpr f32 f32max = 3.402823466e+38F;

	constexpr f32 f32min = 1.175494351e-38F;

	constexpr f64 f64max = 1.7976931348623158e+308;

	constexpr f64 f64min = 2.2250738585072014e-308;

	static inline const f64 infinity = 1.0 / 0.0;

	static inline const f64 nan = 0.0 / 0.0;

	// Is returned by resizable containers as an error.
	constexpr size nullindex = u64max;
}
#pragma once

#include "MetaProgram/MetaProgram.hpp"

#include <new>

namespace ACTL {
	static inline void* alloc(size bytes) noexcept {
		return ::operator new((unsigned)bytes);
	}

	static inline void dealloc(void* data) noexcept {
		::operator delete(data);
	}
}
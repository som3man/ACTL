#pragma once

#include "../Defines.hpp"

namespace ACTL {
	// Struct for two instances of a same or different types.
	template <typename TypeA, typename TypeB = TypeA>
	struct Pair {
		TypeA first; TypeB second;
	};
}
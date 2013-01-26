/***
 * Demonstrike Core
 */

#pragma once

#include "g3dmath.h"

namespace G3D
{

	/** Limited functionality 128-bit unsigned integer. This is primarily to support FNV hashing and other
	cryptography applications.  See the GMP library for high-precision C++ math support. */
	class uint128
	{
	public:

		uint64 hi;
		uint64 lo;

		uint128(const uint64& lo);
		uint128(const uint64& hi, const uint64& lo);
		uint128& operator+=(const uint128& x);
		uint128& operator*=(const uint128& x);
		uint128& operator^=(const uint128& x);
		uint128& operator&=(const uint128& x);
		uint128& operator|=(const uint128& x);
		bool operator==(const uint128& x);
		uint128& operator>>=(const int x);
		uint128& operator<<=(const int x);
		uint128 operator&(const uint128& x);
	};
}

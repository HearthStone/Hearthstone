/***
 * Demonstrike Core
 */

#pragma once

#include "platform.h"

/** Default implementation of EqualsTrait.
	@see G3D::Table for specialization requirements.
*/
template<typename Key> struct EqualsTrait {
	static bool equals(const Key& a, const Key& b) {
		return a == b;
	}
};

/***
 * Demonstrike Core
 */

#pragma once

#include "platform.h"
#include "Crypto.h"
#include "g3dmath.h"
#include "uint128.h"

/** Must be specialized for custom types.
	@see G3D::Table for specialization requirements.
*/
template <typename T> struct HashTrait{};

template <typename T> struct HashTrait<T*> {
	static size_t hashCode(const void* k) { return reinterpret_cast<size_t>(k); }
};

#if 0
template <> struct HashTrait <int> {
	static size_t hashCode(int k) { return static_cast<size_t>(k); }
};
#endif

template <> struct HashTrait <int16> {
	static size_t hashCode(int16 k) { return static_cast<size_t>(k); }
};

template <> struct HashTrait <uint16> {
	static size_t hashCode(uint16 k) { return static_cast<size_t>(k); }
};

//template <> struct HashTrait <int> {
//	static size_t hashCode(int k) { return static_cast<size_t>(k); }
//};

template <> struct HashTrait <int32> {
	static size_t hashCode(int32 k) { return static_cast<size_t>(k); }
};

template <> struct HashTrait <uint32> {
	static size_t hashCode(uint32 k) { return static_cast<size_t>(k); }
};

#if 0
template <> struct HashTrait <long unsigned int> {
	static size_t hashCode(uint32 k) { return static_cast<size_t>(k); }
};
#endif

template <> struct HashTrait <int64> {
	static size_t hashCode(int64 k) { return static_cast<size_t>(k); }
};

template <> struct HashTrait <uint64> {
	static size_t hashCode(uint64 k) { return static_cast<size_t>(k); }
};

template <> struct HashTrait <std::string> {
	static size_t hashCode(const std::string& k) { return static_cast<size_t>(G3D::Crypto::crc32(k.c_str(), k.size())); }
};

template <> struct HashTrait<G3D::uint128> {
	// Use the FNV-1 hash (http://isthe.com/chongo/tech/comp/fnv/#FNV-1).
	static size_t hashCode(G3D::uint128 key) {
		static const G3D::uint128 FNV_PRIME_128(1 << 24, 0x159);
		static const G3D::uint128 FNV_OFFSET_128(0xCF470AAC6CB293D2ULL, 0xF52F88BF32307F8FULL);

		G3D::uint128 hash = FNV_OFFSET_128;
		G3D::uint128 mask(0, 0xFF);
		for (int i = 0; i < 16; ++i) {
			hash *= FNV_PRIME_128;
			hash ^= (mask & key);
			key >>= 8;
		}
	
		uint64 foldedHash = hash.hi ^ hash.lo;
		return static_cast<size_t>((foldedHash >> 32) ^ (foldedHash & 0xFFFFFFFF));
	}
};

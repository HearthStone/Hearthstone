/***
 * Demonstrike Core
 */

#pragma once

#include "platform.h"
#include "g3dmath.h"
#include "HashTrait.h"

namespace G3D {

/**
 \ Vector3int32
 A Vector3 that packs its fields into uint32s.
 */
G3D_BEGIN_PACKED_CLASS(4)
class Vector3int32 {
private:
	// Hidden operators
	bool operator<(const Vector3int32&) const;
	bool operator>(const Vector3int32&) const;
	bool operator<=(const Vector3int32&) const;
	bool operator>=(const Vector3int32&) const;

public:
	int32			  x;
	int32			  y;
	int32			  z;

	Vector3int32() : x(0), y(0), z(0) {}
	Vector3int32(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
	Vector3int32(const class Vector3int16& v);
	Vector3int32(const class Vector3& v);

	inline int32& operator[] (int i) {
		ASSERT(i <= 2);
		return ((int32*)this)[i];
	}

	inline const int32& operator[] (int i) const {
		ASSERT(i <= 2);
		return ((int32*)this)[i];
	}

	inline Vector3int32 operator+(const Vector3int32& other) const {
		return Vector3int32(x + other.x, y + other.y, z + other.z);
	}

	inline Vector3int32 operator-(const Vector3int32& other) const {
		return Vector3int32(x - other.x, y - other.y, z - other.z);
	}

	inline Vector3int32 operator*(const Vector3int32& other) const {
		return Vector3int32(x * other.x, y * other.y, z * other.z);
	}

	inline Vector3int32 operator*(const int s) const {
		return Vector3int32(x * s, y * s, z * s);
	}

	inline Vector3int32& operator+=(const Vector3int32& other) {
		x += other.x;
		y += other.y;
		z += other.y;
		return *this;
	}

	inline Vector3int32& operator-=(const Vector3int32& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	inline Vector3int32& operator*=(const Vector3int32& other) {
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}

	inline bool operator== (const Vector3int32& rkVector) const {
		return ( x == rkVector.x && y == rkVector.y && z == rkVector.z );
	}

	inline bool operator!= (const Vector3int32& rkVector) const {
		return ( x != rkVector.x || y != rkVector.y || z != rkVector.z );
	}

	Vector3int32 max(const Vector3int32& v) const {
		return Vector3int32(iMax(x, v.x), iMax(y, v.y), iMax(z, v.z));
	}

	Vector3int32 min(const Vector3int32& v) const {
		return Vector3int32(iMin(x, v.x), iMin(y, v.y), iMin(z, v.z));
	}

	std::string toString() const;
}
G3D_END_PACKED_CLASS(4)

}

template <> struct HashTrait<G3D::Vector3int32> {
	static size_t hashCode(const G3D::Vector3int32& key) {
		// Mask for the top bit of a uint32
		const uint32 top = (1UL << 31);
		// Mask for the bottom 10 bits of a uint32
		const uint32 bot = 0x000003FF;
		return static_cast<size_t>(((key.x & top) | ((key.y & top) >> 1) | ((key.z & top) >> 2)) | 
								   (((key.x & bot) << 19) ^ ((key.y & bot) << 10) ^ (key.z & bot)));
	}
};

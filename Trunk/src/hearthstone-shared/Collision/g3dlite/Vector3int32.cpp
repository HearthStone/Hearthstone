/***
 * Demonstrike Core
 */

#include "platform.h"
#include "g3dmath.h"
#include "Vector3int32.h"
#include "Vector3int16.h"
#include "Vector3.h"
#include "../../format.h"

namespace G3D {

Vector3int32::Vector3int32(const class Vector3& v) {
	x = (int32)iFloor(v.x + 0.5);
	y = (int32)iFloor(v.y + 0.5);
	z = (int32)iFloor(v.z + 0.5);
}


Vector3int32::Vector3int32(const class Vector3int16& v) {
	x = v.x;
	y = v.y;
	z = v.z;
}

std::string Vector3int32::toString() const {
	return format("(%d, %d, %d)", x, y, z);
}

}

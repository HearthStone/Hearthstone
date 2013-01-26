/***
 * Demonstrike Core
 */

#pragma once

#include "platform.h"
#include "Vector3.h"

namespace G3D {

class Plane;

/**
 An infinite 3D line.
 */
class Line {
protected:

	Vector3 _point;
	Vector3 _direction;

	Line(const Vector3& point, const Vector3& direction) {
		_point	 = point;
		_direction = direction.direction();
	}

public:

	/** Undefined (provided for creating Array<Line> only) */
	inline Line() {}
	virtual ~Line() {}

	/**
	  Constructs a line from two (not equal) points.
	 */
	static Line fromTwoPoints(const Vector3 &point1, const Vector3 &point2) {
		return Line(point1, point2 - point1);
	}

	/**
	  Creates a line from a point and a (nonzero) direction.
	 */
	static Line fromPointAndDirection(const Vector3& point, const Vector3& direction) {
		return Line(point, direction);
	}

	/**
	  Returns the closest point on the line to point.
	 */
	Vector3 closestPoint(const Vector3& pt) const;

	/**
	  Returns the distance between point and the line
	 */
	double distance(const Vector3& point) const {
		return (closestPoint(point) - point).magnitude();
	}

	/** Returns a point on the line */
	Vector3 point() const;

	/** Returns the direction (or negative direction) of the line */
	Vector3 direction() const;

	/**
	 Returns the point where the line and plane intersect.  If there
	 is no intersection, returns a point at infinity.
	 */
	Vector3 intersection(const Plane &plane) const;


	/** Finds the closest point to the two lines.
		
		@param minDist Returns the minimum distance between the lines.

		@cite http://objectmix.com/graphics/133793-coordinates-closest-points-pair-skew-lines.html
	*/
	Vector3 closestPoint(const Line& B, float& minDist) const;

	inline Vector3 closestPoint(const Line& B) const {
		float m;
		return closestPoint(B, m);
	}
};

};// namespace

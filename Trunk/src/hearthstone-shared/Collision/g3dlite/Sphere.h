/***
 * Demonstrike Core
 */

#pragma once

#include "platform.h"
#include "Vector3.h"
#include "Array.h"
#include "Sphere.h"

namespace G3D {

/**
 Sphere.
 */
class Sphere {
private:

	static int32	 dummy;

public:
	Vector3		  center;
	float			radius;

	Sphere() {
		center = Vector3::zero();
		radius = 0;
	}

	Sphere(
		const Vector3&  center,
		float		   radius) {

		this->center = center;
		this->radius = radius;
	}

	virtual ~Sphere() {}

	bool operator==(const Sphere& other) const {
		return (center == other.center) && (radius == other.radius);
	}

	bool operator!=(const Sphere& other) const {
		return !((center == other.center) && (radius == other.radius));
	}

	/**
	 Returns true if point is less than or equal to radius away from
	 the center.
	 */
	bool contains(const Vector3& point) const;

	bool contains(const Sphere& other) const;

	/**
	   @deprecated Use culledBy(Array<Plane>&)
	 */
	bool culledBy(
				  const class Plane*  plane,
				  int				 numPlanes,
				  int32&			  cullingPlaneIndex,
				  const uint32		testMask,
				  uint32&			 childMask) const;
	
	/**
	   @deprecated Use culledBy(Array<Plane>&)
	 */
	bool culledBy(
				  const class Plane*  plane,
				  int				 numPlanes,
				  int32&			  cullingPlaneIndex = dummy,
				  const uint32		testMask = 0xFFFFFFFF) const;

	/**
	   See AABox::culledBy
	*/
	bool culledBy(
				  const Array<Plane>&		plane,
				  int32&					cullingPlaneIndex,
				  const uint32  			testMask,
				  uint32&				 childMask) const;
	
	/**
	 Conservative culling test that does not produce a mask for children.
	 */
	bool culledBy(
				  const Array<Plane>&		plane,
				  int32&					cullingPlaneIndex = dummy,
				  const uint32  			testMask		  = 0xFFFFFFFF) const;

	virtual std::string toString() const;

	float volume() const;

	float area() const;

	/**
	 Uniformly distributed on the surface.
	 */
	Vector3 randomSurfacePoint() const;

	/**
	 Uniformly distributed on the interior (includes surface)
	 */
	Vector3 randomInteriorPoint() const;

	void getBounds(class AABox& out) const;

	bool intersects(const Sphere& other) const;

	/** Translates the sphere */
	Sphere operator+(const Vector3& v) const {
		return Sphere(center + v, radius);
	}

	/** Translates the sphere */
	Sphere operator-(const Vector3& v) const {
		return Sphere(center - v, radius);
	}

	/** Sets this to the smallest sphere that encapsulates both */
	void merge(const Sphere& s);
};

}

template <> struct HashTrait<G3D::Sphere> {
	static size_t hashCode(const G3D::Sphere& key) { 
		return static_cast<size_t>(key.center.hashCode() + (key.radius * 13));
	}
};

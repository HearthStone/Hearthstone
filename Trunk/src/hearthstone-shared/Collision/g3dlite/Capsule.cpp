/***
 * Demonstrike Core
 */

#include "Capsule.h"
#include "LineSegment.h"
#include "Sphere.h"
#include "CoordinateFrame.h"
#include "Line.h"
#include "AABox.h"

namespace G3D {

Capsule::Capsule() {
}


Capsule::Capsule(const Vector3& _p1, const Vector3& _p2, float _r) 
	: p1(_p1), p2(_p2), _radius(_r) {
}


Line Capsule::axis() const {
	return Line::fromTwoPoints(p1, p2);
}


float Capsule::volume() const {
	return 
		// Sphere volume
		pow(_radius, 3) * pi() * 4 / 3 +

		// Cylinder volume
		pow(_radius, 2) * (p1 - p2).magnitude();
}


float Capsule::area() const {

	return
		// Sphere area
		pow(_radius, 2) * 4 * pi() +

		// Cylinder area
		twoPi() * _radius * (p1 - p2).magnitude();
}


void Capsule::getBounds(AABox& out) const {
	Vector3 min = p1.min(p2) - (Vector3(1, 1, 1) * _radius);
	Vector3 max = p1.max(p2) + (Vector3(1, 1, 1) * _radius);

	out = AABox(min, max);
}


bool Capsule::contains(const Vector3& p) const { 
	return LineSegment::fromTwoPoints(p1, p2).distanceSquared(p) <= square(radius());
}


void Capsule::getRandomSurfacePoint(Vector3& p, Vector3& N) const {
	float h = height();
	float r = radius();

	// Create a random point on a standard capsule and then rotate to the global frame.

	// Relative areas
	float capRelArea  = sqrt(r) / 2.0f;
	float sideRelArea = r * h;

	float r1 = uniformRandom(0, capRelArea * 2 + sideRelArea);

	if (r1 < capRelArea * 2) {

		// Select a point uniformly at random on a sphere
		N = Sphere(Vector3::zero(), 1).randomSurfacePoint();
		p = N * r;
		p.y += sign(p.y) * h / 2.0f;
	} else {
		// Side
		float a = uniformRandom(0, (float)twoPi());
		N.x = cos(a);
		N.y = 0;
		N.z = sin(a);
		p.x = N.x * r;
		p.z = N.y * r;
		p.y = uniformRandom(-h / 2.0f, h / 2.0f);
	}

	// Transform to world space
	CoordinateFrame cframe;
	getReferenceFrame(cframe);
	
	p = cframe.pointToWorldSpace(p);
	N = cframe.normalToWorldSpace(N);
}


void Capsule::getReferenceFrame(CoordinateFrame& cframe) const {
	cframe.translation = center();

	Vector3 Y = (p1 - p2).direction();
	Vector3 X = (abs(Y.dot(Vector3::unitX())) > 0.9) ? Vector3::unitY() : Vector3::unitX();
	Vector3 Z = X.cross(Y).direction();
	X = Y.cross(Z);		
	cframe.rotation.setColumn(0, X);
	cframe.rotation.setColumn(1, Y);
	cframe.rotation.setColumn(2, Z);
}


Vector3 Capsule::randomInteriorPoint() const {
	float h = height();
	float r = radius();

	// Create a random point in a standard capsule and then rotate to the global frame.

	Vector3 p;

	float hemiVolume = pi() * (r*r*r) * 4 / 6.0;
	float cylVolume = pi() * square(r) * h;
	
	float r1 = uniformRandom(0, 2.0 * hemiVolume + cylVolume);

	if (r1 < 2.0 * hemiVolume) {

		p = Sphere(Vector3::zero(), r).randomInteriorPoint();

		p.y += sign(p.y) * h / 2.0f;

	} else {

		// Select a point uniformly at random on a disk
		float a = uniformRandom(0, (float)twoPi());
		float r2 = sqrt(uniformRandom(0, 1)) * r;

		p = Vector3(cos(a) * r2,
					uniformRandom(-h / 2.0f, h / 2.0f),
					sin(a) * r2);
	}

	// Transform to world space
	CoordinateFrame cframe;
	getReferenceFrame(cframe);
	
	return cframe.pointToWorldSpace(p);
}

} // namespace

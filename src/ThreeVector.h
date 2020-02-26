#ifndef THREEVECTOR_H
#define THREEVECTOR_H

#include <iostream>
#include <cmath>

class ThreeVector {

public:
	ThreeVector();
	explicit ThreeVector(double x);
	ThreeVector(double x, double y);
	ThreeVector(double x, double y, double z);
	// The constructor.  

	inline ThreeVector(const ThreeVector &);
	// The copy constructor.

	~ThreeVector() {}
	// The destructor.  Not virtual - inheritance from this class is dangerous.

	inline double x() const;
	inline double y() const;
	inline double z() const;
	// The components in cartesian coordinate system.  Same as getX() etc.

	inline void setX(double);
	inline void setY(double);
	inline void setZ(double);
	// Set the components in cartesian coordinate system.

	inline void set(double x, double y, double z);
	// Set all three components in cartesian coordinate system.

	inline double mag2() const;
	// The magnitude squared (r^2 in spherical coordinate system).

	inline double mag() const;
	// The magnitude (r in spherical coordinate system).

	void setMag(double);
	// Set magnitude keeping theta and phi constant (BaBar).

	inline ThreeVector & operator = (const ThreeVector &);
	// Assignment.

	inline bool operator == (const ThreeVector &) const;
	inline bool operator != (const ThreeVector &) const;
	// Comparisons (Geant4). 

	inline ThreeVector & operator += (const ThreeVector &);
	// Addition.

	inline ThreeVector & operator -= (const ThreeVector &);
	// Subtraction.

	inline ThreeVector operator - () const;
	// Unary minus.

	inline ThreeVector & operator *= (double);
	// Scaling with real numbers.

	ThreeVector & operator /= (double);
	// Division by (non-zero) real number.

	inline ThreeVector unit() const;
	// Vector parallel to this, but of length 1.

	inline double dot(const ThreeVector &) const;
	// double product.

	inline ThreeVector cross(const ThreeVector &) const;
	// Cross product.


	inline double getX() const;
	inline double getY() const;
	inline double getZ() const;
	// x(), y(), and z()

	inline double r() const;
	// mag()

	int compare(const ThreeVector & v) const;
	bool operator > (const ThreeVector & v) const;
	bool operator < (const ThreeVector & v) const;
	bool operator>= (const ThreeVector & v) const;
	bool operator<= (const ThreeVector & v) const;
	// dictionary ordering according to z, then y, then x component

protected:

	double dx;
	double dy;
	double dz;
};  // ThreeVector

// Return a new vector based on a rotation of the supplied vector

std::ostream & operator << (std::ostream &, const ThreeVector &);
// Output to a stream.

std::istream & operator >> (std::istream &, ThreeVector &);
// Input from a stream.

ThreeVector operator / (const ThreeVector &, double a);
// Division of 3-vectors by non-zero real number

inline ThreeVector operator + (const ThreeVector &, const ThreeVector &);
// Addition of 3-vectors.

inline ThreeVector operator - (const ThreeVector &, const ThreeVector &);
// Subtraction of 3-vectors.

inline double operator * (const ThreeVector &, const ThreeVector &);
// double product of 3-vectors.

inline ThreeVector operator * (const ThreeVector &, double a);
inline ThreeVector operator * (double a, const ThreeVector &);
// Scaling of 3-vectors with a real number

// ------------------
// Access to elements
// ------------------

// x, y, z

inline double ThreeVector::x() const { return dx; }
inline double ThreeVector::y() const { return dy; }
inline double ThreeVector::z() const { return dz; }

inline double ThreeVector::getX() const { return dx; }
inline double ThreeVector::getY() const { return dy; }
inline double ThreeVector::getZ() const { return dz; }

inline void ThreeVector::setX(double x1) { dx = x1; }
inline void ThreeVector::setY(double y1) { dy = y1; }
inline void ThreeVector::setZ(double z1) { dz = z1; }

inline void ThreeVector::set(double x1, double y1, double z1) {
	dx = x1;
	dy = y1;
	dz = z1;
}

// ------------
// Constructors
// ------------

inline ThreeVector::ThreeVector()
	: dx(0.), dy(0.), dz(0.) {}
inline ThreeVector::ThreeVector(double x1)
	: dx(x1), dy(0.), dz(0.) {}
inline ThreeVector::ThreeVector(double x1, double y1)
	: dx(x1), dy(y1), dz(0.) {}
inline ThreeVector::ThreeVector(double x1, double y1, double z1)
	: dx(x1), dy(y1), dz(z1) {}

inline ThreeVector::ThreeVector(const ThreeVector & p)
	: dx(p.dx), dy(p.dy), dz(p.dz) {}


inline ThreeVector & ThreeVector::operator = (const ThreeVector & p) {
	dx = p.dx;
	dy = p.dy;
	dz = p.dz;
	return *this;
}


// --------------
// Global methods
// --------------

inline ThreeVector operator + (const ThreeVector & a, const ThreeVector & b) {
	return ThreeVector(a.x() + b.x(), a.y() + b.y(), a.z() + b.z());
}

inline ThreeVector operator - (const ThreeVector & a, const ThreeVector & b) {
	return ThreeVector(a.x() - b.x(), a.y() - b.y(), a.z() - b.z());
}

inline ThreeVector operator * (const ThreeVector & p, double a) {
	return ThreeVector(a*p.x(), a*p.y(), a*p.z());
}

inline ThreeVector operator * (double a, const ThreeVector & p) {
	return ThreeVector(a*p.x(), a*p.y(), a*p.z());
}

inline double operator * (const ThreeVector & a, const ThreeVector & b) {
	return a.dot(b);
}

// ------------------
// Access to elements
// ------------------

// r, theta, phi

inline double ThreeVector::mag2() const { return dx * dx + dy * dy + dz * dz; }
inline double ThreeVector::mag()  const { return std::sqrt(mag2()); }
inline double ThreeVector::r()    const { return mag(); }


// ----------
// Comparison
// ----------

inline bool ThreeVector::operator == (const ThreeVector& v) const {
	return (v.x() == x() && v.y() == y() && v.z() == z()) ? true : false;
}

inline bool ThreeVector::operator != (const ThreeVector& v) const {
	return (v.x() != x() || v.y() != y() || v.z() != z()) ? true : false;
}

// ----------
// Arithmetic
// ----------

inline ThreeVector& ThreeVector::operator += (const ThreeVector & p) {
	dx += p.x();
	dy += p.y();
	dz += p.z();
	return *this;
}

inline ThreeVector& ThreeVector::operator -= (const ThreeVector & p) {
	dx -= p.x();
	dy -= p.y();
	dz -= p.z();
	return *this;
}

inline ThreeVector ThreeVector::operator - () const {
	return ThreeVector(-dx, -dy, -dz);
}

inline ThreeVector& ThreeVector::operator *= (double a) {
	dx *= a;
	dy *= a;
	dz *= a;
	return *this;
}

// -------------------
// Combine two Vectors
// -------------------

inline double ThreeVector::dot(const ThreeVector & p) const {
	return dx * p.x() + dy * p.y() + dz * p.z();
}

inline ThreeVector ThreeVector::cross(const ThreeVector & p) const {
	return ThreeVector(dy*p.z() - p.y()*dz, dz*p.x() - p.z()*dx, dx*p.y() - p.x()*dy);
}

// ----------
// Properties
// ----------

inline ThreeVector ThreeVector::unit() const {
	double  tot = mag2();
	ThreeVector p(x(), y(), z());
	return tot > 0.0 ? p *= (1.0 / std::sqrt(tot)) : p;
}

#endif /*THREEVECTOR_H */

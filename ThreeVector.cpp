#include "ThreeVector.h"
#include <cmath>
#include <iostream>



std::ostream & operator<< (std::ostream & os, const ThreeVector & v) {
	return os  << v.x() << "  " << v.y() << "  " << v.z() ;
}


std::istream & operator>>(std::istream & is, ThreeVector & v) {
	double x, y, z;
	is >> x >> y >> z;
	v.set(x, y, z);
	return  is;
}  // operator>>()

ThreeVector operator/  (const ThreeVector & v1, double c) {
	double   oneOverC = 1.0 / c;
	return ThreeVector(v1.x() * oneOverC,
		v1.y() * oneOverC,
		v1.z() * oneOverC);
} /* v / c */

ThreeVector & ThreeVector::operator/= (double c) {
	double oneOverC = 1.0 / c;
	dx *= oneOverC;
	dy *= oneOverC;
	dz *= oneOverC;
	return *this;
}





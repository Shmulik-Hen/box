#ifndef _VECTOR_H
#define _VECTOR_H
#include "unit.h"
#include <iostream.h>

enum coords
{
	X,
	Y,
	Z
};
class matrix;
class vector
{
public:

	unit coord[3];
	vector();
	~vector();
	// vector(long,long,long);
	vector(unit, unit, unit);
	vector operator+(const vector &);
	vector operator-(const vector &);
	vector &operator+=(const vector &);
	vector &operator-=(const vector &);
	unit operator*(const vector &);
	friend void normalize(vector &);
	friend vector operator*(const matrix &, const vector &);
	friend vector project(const vector &, const vector &);
	friend ostream &operator<<(ostream &, const vector &);
	friend istream &operator>>(istream &, vector &);
	void read(ifstream &);
};
#endif

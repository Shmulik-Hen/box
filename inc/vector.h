#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <fstream>
#include "unit.h"

using std::ifstream;
using std::istream;
using std::ostream;

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

#endif //__VECTOR_H__

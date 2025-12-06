#ifndef __ATTRIB_H__
#define __ATTRIB_H__

#include <iostream>
#include "unit.h"

using std::ifstream;
using std::istream;
using std::ostream;

class matrix;
class attrib
{
	unit deg_x, deg_y, deg_z;
	unit off_x, off_y, off_z;
	unit zoom;

public:

	attrib();
	~attrib();
	// attrib(long,long,long,long,long,long,long);
	attrib(unit, unit, unit, unit, unit, unit, unit);
	friend attrib operator+(const attrib &, const attrib &);
	attrib &operator+=(const attrib &);
	friend void prep_gen_mat(matrix &, const attrib &);
	friend void prep_rot_mat(matrix &, const attrib &);
	friend ostream &operator<<(ostream &, const attrib &);
	friend istream &operator>>(istream &, attrib &);
	void read(ifstream &);
};

#endif //__ATTRIB_H__

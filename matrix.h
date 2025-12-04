#ifndef _MATRIX_H
#define _MATRIX_H
#include "unit.h"
#include "attrib.h"
#include "vector.h"

class matrix
{
	unit mat[4][4];

public:

	matrix();
	~matrix();
	matrix(unit[4][4]);
	matrix &operator*=(const matrix &);
	friend matrix operator*(const matrix &a, const matrix &b);
	friend void prep_gen_mat(matrix &, const attrib &);
	friend void prep_rot_mat(matrix &, const attrib &);
	friend vector operator*(const matrix &, const vector &);
};
#endif

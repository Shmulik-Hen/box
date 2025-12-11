#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "common.h"
#include "attrib.h"
#include "my_vector.h"
#include "unit.h"

namespace matrix_ns
{

using attrib_ns::attrib;
using my_vector_ns::my_vector;
using unit_ns::unit;

const int dim = 4;

class matrix : public attrib,
	       public my_vector
{
	unit mat[dim][dim];

public:

	matrix();
	matrix(unit[dim][dim]);
	~matrix();
	matrix operator*(const matrix &);
	matrix &operator*=(const matrix &);
	my_vector operator*(const my_vector &);
	void prep_gen_mat(const attrib &);
	void prep_rot_mat(const attrib &);
};

} // namespace matrix_ns
#endif //__MATRIX_H__

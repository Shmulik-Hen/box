#include "matrix.h"

namespace matrix_ns
{

// clang-format off
unit zero[dim][dim] = {
	{ ZERO, ZERO, ZERO, ZERO  },
	{ ZERO, ZERO, ZERO, ZERO  },
	{ ZERO, ZERO, ZERO, ZERO  },
	{ ZERO, ZERO, ZERO, ZERO  }
};

unit unt[dim][dim] = {
	{ UNIT, ZERO, ZERO, ZERO  },
	{ ZERO, UNIT, ZERO, ZERO  },
	{ ZERO, ZERO, UNIT, ZERO  },
	{ ZERO, ZERO, ZERO, UNIT  }
};
// clang-format on

matrix::matrix()
{
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			mat[i][j] = zero[i][j];
		}
	}
}

matrix::matrix(unit n[dim][dim])
{
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			mat[i][j] = n[i][j];
		}
	}
}

matrix::~matrix()
{
}

matrix matrix::operator*(const matrix &m)
{
	matrix temp;
	temp.mat[0][0] = mat[0][0] * m.mat[0][0] + mat[0][1] * m.mat[1][0] + mat[0][2] * m.mat[2][0] + mat[0][3] * m.mat[3][0];
	temp.mat[0][1] = mat[0][0] * m.mat[0][1] + mat[0][1] * m.mat[1][1] + mat[0][2] * m.mat[2][1] + mat[0][3] * m.mat[3][1];
	temp.mat[0][2] = mat[0][0] * m.mat[0][2] + mat[0][1] * m.mat[1][2] + mat[0][2] * m.mat[2][2] + mat[0][3] * m.mat[3][2];
	temp.mat[0][3] = mat[0][0] * m.mat[0][3] + mat[0][1] * m.mat[1][3] + mat[0][2] * m.mat[2][3] + mat[0][3] * m.mat[3][3];
	temp.mat[1][0] = mat[1][0] * m.mat[0][0] + mat[1][1] * m.mat[1][0] + mat[1][2] * m.mat[2][0] + mat[1][3] * m.mat[3][0];
	temp.mat[1][1] = mat[1][0] * m.mat[0][1] + mat[1][1] * m.mat[1][1] + mat[1][2] * m.mat[2][1] + mat[1][3] * m.mat[3][1];
	temp.mat[1][2] = mat[1][0] * m.mat[0][2] + mat[1][1] * m.mat[1][2] + mat[1][2] * m.mat[2][2] + mat[1][3] * m.mat[3][2];
	temp.mat[1][3] = mat[1][0] * m.mat[0][3] + mat[1][1] * m.mat[1][3] + mat[1][2] * m.mat[2][3] + mat[1][3] * m.mat[3][3];
	temp.mat[2][0] = mat[2][0] * m.mat[0][0] + mat[2][1] * m.mat[1][0] + mat[2][2] * m.mat[2][0] + mat[2][3] * m.mat[3][0];
	temp.mat[2][1] = mat[2][0] * m.mat[0][1] + mat[2][1] * m.mat[1][1] + mat[2][2] * m.mat[2][1] + mat[2][3] * m.mat[3][1];
	temp.mat[2][2] = mat[2][0] * m.mat[0][2] + mat[2][1] * m.mat[1][2] + mat[2][2] * m.mat[2][2] + mat[2][3] * m.mat[3][2];
	temp.mat[2][3] = mat[2][0] * m.mat[0][3] + mat[2][1] * m.mat[1][3] + mat[2][2] * m.mat[2][3] + mat[2][3] * m.mat[3][3];
	temp.mat[3][0] = mat[3][0] * m.mat[0][0] + mat[3][1] * m.mat[1][0] + mat[3][2] * m.mat[2][0] + mat[3][3] * m.mat[3][0];
	temp.mat[3][1] = mat[3][0] * m.mat[0][1] + mat[3][1] * m.mat[1][1] + mat[3][2] * m.mat[2][1] + mat[3][3] * m.mat[3][1];
	temp.mat[3][2] = mat[3][0] * m.mat[0][2] + mat[3][1] * m.mat[1][2] + mat[3][2] * m.mat[2][2] + mat[3][3] * m.mat[3][2];
	temp.mat[3][3] = mat[3][0] * m.mat[0][3] + mat[3][1] * m.mat[1][3] + mat[3][2] * m.mat[2][3] + mat[3][3] * m.mat[3][3];
	return temp;
}

matrix &matrix::operator*=(const matrix &m)
{
	*this = *this * m;
	return *this;
}

my_vector matrix::operator*(const my_vector &v)
{
	my_vector temp;
	temp.get_coord(coord::X) = mat[0][0] * v.get_coord(coord::X) + mat[0][1] * v.get_coord(coord::Y) + mat[0][2] * v.get_coord(coord::Z) + mat[0][3];
	temp.get_coord(coord::Y) = mat[1][0] * v.get_coord(coord::X) + mat[1][1] * v.get_coord(coord::Y) + mat[1][2] * v.get_coord(coord::Z) + mat[1][3];
	temp.get_coord(coord::Z) = mat[2][0] * v.get_coord(coord::X) + mat[2][1] * v.get_coord(coord::Y) + mat[2][2] * v.get_coord(coord::Z) + mat[2][3];
	return temp;
}

void matrix::prep_gen_mat(const attrib &a)
{
	mat[0][0] = unit(a.get_zoom() * cos(a.get_deg_y()) * cos(a.get_deg_z()));
	mat[0][1] = a.get_zoom() * -cos(a.get_deg_y()) * sin(a.get_deg_z());
	mat[0][2] = a.get_zoom() * -sin(a.get_deg_y());
	mat[0][3] = a.get_off_x();
	mat[1][0] = a.get_zoom() * (cos(a.get_deg_x()) * sin(a.get_deg_z()) - sin(a.get_deg_x()) * sin(a.get_deg_y()) * cos(a.get_deg_z()));
	mat[1][1] = a.get_zoom() * (cos(a.get_deg_x()) * cos(a.get_deg_z()) + sin(a.get_deg_x()) * sin(a.get_deg_y()) * sin(a.get_deg_z()));
	mat[1][2] = a.get_zoom() * -sin(a.get_deg_x()) * cos(a.get_deg_y());
	mat[1][3] = a.get_off_y();
	mat[2][0] = a.get_zoom() * (sin(a.get_deg_x()) * sin(a.get_deg_z()) + cos(a.get_deg_x()) * sin(a.get_deg_y()) * cos(a.get_deg_z()));
	mat[2][1] = a.get_zoom() * (sin(a.get_deg_x()) * cos(a.get_deg_z()) - cos(a.get_deg_x()) * sin(a.get_deg_y()) * sin(a.get_deg_z()));
	mat[2][2] = a.get_zoom() * cos(a.get_deg_x()) * cos(a.get_deg_y());
	mat[2][3] = a.get_off_z();
	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = UNIT;
}

void matrix::prep_rot_mat(const attrib &a)
{
	mat[0][0] = cos(a.get_deg_y()) * cos(a.get_deg_z());
	mat[0][1] = -cos(a.get_deg_y()) * sin(a.get_deg_z());
	mat[0][2] = -sin(a.get_deg_y());
	mat[0][3] = 0;
	mat[1][0] = cos(a.get_deg_x()) * sin(a.get_deg_z()) - sin(a.get_deg_x()) * sin(a.get_deg_y()) * cos(a.get_deg_z());
	mat[1][1] = cos(a.get_deg_x()) * cos(a.get_deg_z()) + sin(a.get_deg_x()) * sin(a.get_deg_y()) * sin(a.get_deg_z());
	mat[1][2] = -sin(a.get_deg_x()) * cos(a.get_deg_y());
	mat[1][3] = 0;
	mat[2][0] = sin(a.get_deg_x()) * sin(a.get_deg_z()) + cos(a.get_deg_x()) * sin(a.get_deg_y()) * cos(a.get_deg_z());
	mat[2][1] = sin(a.get_deg_x()) * cos(a.get_deg_z()) - cos(a.get_deg_x()) * sin(a.get_deg_y()) * sin(a.get_deg_z());
	mat[2][2] = cos(a.get_deg_x()) * cos(a.get_deg_y());
	mat[2][3] = 0;
	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = UNIT;
}

matrix get_unit_mat()
{
	return matrix(unt);
}
} // namespace matrix_ns

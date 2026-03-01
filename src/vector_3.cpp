#include <cmath>

// #define DEBUG_PRINTS
#include "vector_3.h"
#include "utils.h"

namespace vector_3_ns
{

vector_3::vector_3()
{
	_coords[X_] = ZERO;
	_coords[Y_] = ZERO;
	_coords[Z_] = ZERO;
}

vector_3::vector_3(const vector_3& v)
{
	_coords[X_] = v._coords[X_];
	_coords[Y_] = v._coords[Y_];
	_coords[Z_] = v._coords[Z_];
}

vector_3::vector_3(const unit v[dim])
{
	_coords[X_] = v[X_];
	_coords[Y_] = v[Y_];
	_coords[Z_] = v[Z_];
}

vector_3::vector_3(const unit x, const unit y, const unit z)
{
	_coords[X_] = x;
	_coords[Y_] = y;
	_coords[Z_] = z;
}

bool vector_3::read(std::ifstream& ifs)
{
	LINE line;

	for (int i = X_; i <= Z_; i++) {
		// clang-format off
		while ((!read_word(ifs, line)) && (!ifs.eof()));
		if (ifs.eof())  return false;
		// clang-format on
		_coords[i] = strtof(line, NULL);
		// clang-format on
	}

	return true;
}

void vector_3::print() const
{
	DBG(STR("  vector:", 1));
	for (int i = X_; i <= Z_; i++) {
		DBG("    i: " << i << PFX << FLT(_coords[i], 3));
	}
}

unit vector_3::get(int i) const
{
	if (is_valid_coord(i)) {
		return _coords[i];
	}

	return ZERO;
}

void vector_3::set(const int i, const unit& u)
{
	if (is_valid_coord(i)) {
		_coords[i] = u;
	}
}

unit vector_3::dot(const vector_3& v1, const vector_3& v2)
{
	return (v1._coords[X_] * v2._coords[X_] +
	        v1._coords[Y_] * v2._coords[Y_] +
	        v1._coords[Z_] * v2._coords[Z_]);
}

vector_3 vector_3::cross(const vector_3& v1, const vector_3& v2)
{
	return {v1._coords[Y_] * v2._coords[Z_] - v1._coords[Z_] * v2._coords[Y_],
	        v1._coords[Z_] * v2._coords[X_] - v1._coords[X_] * v2._coords[Z_],
	        v1._coords[X_] * v2._coords[Y_] - v1._coords[Y_] * v2._coords[X_]};
}

vector_3 vector_3::project(const vector_3& t, const vector_3& v)
{
	vector_3 temp;

	if (std::abs(v._coords[Z_] - t._coords[Z_]) >= UNIT) {
		temp._coords[X_] = ((t._coords[X_] - v._coords[X_]) /
		                    (v._coords[Z_] - t._coords[Z_]) *
		                    v._coords[Z_]);
		temp._coords[Y_] = ((t._coords[Y_] - v._coords[Y_]) /
		                    (v._coords[Z_] - t._coords[Z_]) *
		                    v._coords[Z_]);
	}
	else {
		temp._coords[X_] = ((t._coords[X_] - v._coords[X_]) *
		                    v._coords[Z_] /
		                    (v._coords[Z_] - t._coords[Z_] + UNIT));
		temp._coords[Y_] = ((t._coords[Y_] - v._coords[Y_]) *
		                    v._coords[Z_] /
		                    (v._coords[Z_] - t._coords[Z_] + UNIT));
	}

	return temp;
}

vector_3 vector_3::project(const vector_3& world_point, const vector_3& cam_pos, unit focal_len, unit near_eps, int forward_sign)
{
	unit dx = world_point.get(X_) - cam_pos.get(X_);
	unit dy = world_point.get(Y_) - cam_pos.get(Y_);
	unit dz = world_point.get(Z_) - cam_pos.get(Z_);

	if (near_eps == ZERO) {
		near_eps = UNIT;
	}

	unit depth = (unit)forward_sign * dz; // >0 is in front

	if (depth < near_eps) {
		depth = near_eps;
	}

	vector_3 out;
	out.set(X_, dx * focal_len / depth);
	out.set(Y_, dy * focal_len / depth);
	out.set(Z_, depth);

	return out;
}

vector_3 vector_3::normalize(const vector_3& v)
{
	unit sqr_len = dot(v, v);

	if (sqr_len <= EPSILON * EPSILON) {
		return {ZERO, ZERO, ZERO};
	}

	unit inv_len = UNIT / std::sqrt(sqr_len);
	return v * inv_len;
}

vector_3 vector_3::operator+(const vector_3& v) const
{
	return vector_3(_coords[X_] + v._coords[X_],
	                _coords[Y_] + v._coords[Y_],
	                _coords[Z_] + v._coords[Z_]);
}

vector_3 vector_3::operator-(const vector_3& v) const
{
	return vector_3(_coords[X_] - v._coords[X_],
	                _coords[Y_] - v._coords[Y_],
	                _coords[Z_] - v._coords[Z_]);
}

vector_3 vector_3::operator*(const unit u) const
{
	return vector_3(_coords[X_] * u,
	                _coords[Y_] * u,
	                _coords[Z_] * u);
}

vector_3& vector_3::operator+=(const vector_3& v)
{
	_coords[X_] += v._coords[X_];
	_coords[Y_] += v._coords[Y_];
	_coords[Z_] += v._coords[Z_];

	return *this;
}

vector_3& vector_3::operator-=(const vector_3& v)
{
	_coords[X_] -= v._coords[X_];
	_coords[Y_] -= v._coords[Y_];
	_coords[Z_] -= v._coords[Z_];

	return *this;
}

std::ostream& operator<<(std::ostream& os, const vector_3& v)
{
	return os << '(' << v._coords[X_] << ','
	          << v._coords[Y_] << ','
	          << v._coords[Z_] << ')';
}

std::istream& operator>>(std::istream& is, vector_3& v)
{
	is >> v._coords[X_];
	is >> v._coords[Y_];
	is >> v._coords[Z_];
	return is;
}

} // namespace vector_3_ns

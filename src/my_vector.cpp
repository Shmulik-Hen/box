#include "my_vector.h"

namespace my_vector_ns
{

my_vector::my_vector()
{
	_coords = new vec_entry();
	_coords->clear();
	(*_coords)[coord::X] = unit_ns::ZERO;
	(*_coords)[coord::Y] = unit_ns::ZERO;
	(*_coords)[coord::Z] = unit_ns::ZERO;
}

my_vector::my_vector(unit x, unit y, unit z)
{
	_coords = new vec_entry();
	_coords->clear();
	(*_coords)[coord::X] = x;
	(*_coords)[coord::Y] = y;
	(*_coords)[coord::Z] = z;
}

my_vector::~my_vector()
{
	if (_coords)
		delete _coords;
}

my_vector my_vector::operator+(const my_vector &v) const
{
	return my_vector(_coords->at(coord::X) + v._coords->at(coord::X), _coords->at(coord::Y) + v._coords->at(coord::Y), _coords->at(coord::Z) + v._coords->at(coord::Z));
}

my_vector my_vector::operator-(const my_vector &v) const
{
	return my_vector(_coords->at(coord::X) - v._coords->at(coord::X), _coords->at(coord::Y) - v._coords->at(coord::Y), _coords->at(coord::Z) - v._coords->at(coord::Z));
}

my_vector &my_vector::operator+=(const my_vector &v)
{
	_coords->at(coord::X) += v._coords->at(coord::X);
	_coords->at(coord::Y) += v._coords->at(coord::Y);
	_coords->at(coord::Z) += v._coords->at(coord::Z);
	return *this;
}

my_vector &my_vector::operator-=(const my_vector &v)
{
	_coords->at(coord::X) -= v._coords->at(coord::X);
	_coords->at(coord::Y) -= v._coords->at(coord::Y);
	_coords->at(coord::Z) -= v._coords->at(coord::Z);
	return *this;
}

unit my_vector::operator*(const my_vector &v) const
{
	return unit(_coords->at(coord::X) * v._coords->at(coord::X) + _coords->at(coord::Y) * v._coords->at(coord::Y) + _coords->at(coord::Z) * v._coords->at(coord::Z));
}

unit my_vector::get_coord(coord c) const
{
	unit u = unit_ns::ZERO;
	if (is_valid_coord(c))
		u = _coords->at(c);
	return u;
}

my_vector my_vector::project(const my_vector &p, const my_vector &v)
{
	my_vector temp;
	if (abs(v._coords->at(coord::Z) - p._coords->at(coord::Z)) >= unit_ns::UNIT) {
		temp._coords->at(coord::X) = ((p._coords->at(coord::X) - v._coords->at(coord::X)) / (v._coords->at(coord::Z) - p._coords->at(coord::Z))) * v._coords->at(coord::Z);
		temp._coords->at(coord::Y) = ((p._coords->at(coord::Y) - v._coords->at(coord::Y)) / (v._coords->at(coord::Z) - p._coords->at(coord::Z))) * v._coords->at(coord::Z);
	}
	else {
		temp._coords->at(coord::X) = ((p._coords->at(coord::X) - v._coords->at(coord::X)) * v._coords->at(coord::Z)) / (v._coords->at(coord::Z) - p._coords->at(coord::Z) + unit_ns::UNIT);
		temp._coords->at(coord::Y) = ((p._coords->at(coord::Y) - v._coords->at(coord::Y)) * v._coords->at(coord::Z)) / (v._coords->at(coord::Z) - p._coords->at(coord::Z) + unit_ns::UNIT);
	}
	return temp;
}

void my_vector::normalize(my_vector &v)
{
	unit norm = sqrt(v * v);
	v._coords->at(coord::X) /= norm;
	v._coords->at(coord::Y) /= norm;
	v._coords->at(coord::Z) /= norm;
}

bool my_vector::read(ifstream &f)
{
	printf("my_vector::read \n");
	for (coord c = coord::__first_coord; c < coord::__last_coord; c++) {
		if (!_coords->at(c).read(f)) {
			printf{"vector::read error\n"};
			return false;
		}
	}

	return true;
}

void my_vector::print() const
{
	printf("        vector:\n");
	for (coord c = coord::__first_coord; c < coord::__last_coord; ++c)
		_coords->at(c).print();
}

ostream &operator<<(ostream &o, const my_vector &v)
{
	return o << '(' << v._coords->at(my_vector::coord::X) << ',' << v._coords->at(my_vector::coord::Y) << ',' << v._coords->at(my_vector::coord::Z) << ')';
}

istream &operator>>(istream &i, my_vector &v)
{
	i >> v._coords->at(my_vector::coord::X);
	i >> v._coords->at(my_vector::coord::Y);
	i >> v._coords->at(my_vector::coord::Z);
	return i;
}

bool my_vector::is_valid_coord(coord c) const
{
	return (c >= coord::__first_coord && c < coord::__last_coord);
}

} // namespace my_vector_ns

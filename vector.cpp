#include "vector.h"

vector::vector()
{
}

vector::~vector()
{
}
/*
vector::vector(long x,long y,long z)
{
 coord[X]=unit(x<<10);
 coord[Y]=unit(y<<10);
 coord[Z]=unit(z<<10);
}*/

vector::vector(unit x, unit y, unit z)
{
	coord[X] = x;
	coord[Y] = y;
	coord[Z] = z;
}

vector vector::operator+(const vector &v)
{
	return vector(coord[X] + v.coord[X], coord[Y] + v.coord[Y], coord[Z] + v.coord[Z]);
}

vector vector::operator-(const vector &v)
{
	return vector(coord[X] - v.coord[X], coord[Y] - v.coord[Y], coord[Z] - v.coord[Z]);
}

unit vector::operator*(const vector &v)
{
	return unit(coord[X] * v.coord[X] + coord[Y] * v.coord[Y] + coord[Z] * v.coord[Z]);
}

vector &vector::operator+=(const vector &v)
{
	coord[X] += v.coord[X];
	coord[Y] += v.coord[Y];
	coord[Z] += v.coord[Z];
	return *this;
}

vector &vector::operator-=(const vector &v)
{
	coord[X] -= v.coord[X];
	coord[Y] -= v.coord[Y];
	coord[Z] -= v.coord[Z];
	return *this;
}

void normalize(vector &v)
{
	unit norm = sqrt(v * v);
	v.coord[X] /= norm;
	v.coord[Y] /= norm;
	v.coord[Z] /= norm;
}

vector project(const vector &p, const vector &v)
{
	vector temp;
	if (abs(v.coord[Z] - p.coord[Z]) >= UNIT) {
		temp.coord[X] = ((p.coord[X] - v.coord[X]) / (v.coord[Z] - p.coord[Z])) * v.coord[Z];
		temp.coord[Y] = ((p.coord[Y] - v.coord[Y]) / (v.coord[Z] - p.coord[Z])) * v.coord[Z];
	}
	else {
		temp.coord[X] = ((p.coord[X] - v.coord[X]) * v.coord[Z]) / (v.coord[Z] - p.coord[Z] + UNIT);
		temp.coord[Y] = ((p.coord[Y] - v.coord[Y]) * v.coord[Z]) / (v.coord[Z] - p.coord[Z] + UNIT);
	}
	return temp;
}

ostream &operator<<(ostream &o, const vector &v)
{
	return o << '(' << v.coord[X] << ',' << v.coord[Y] << ',' << v.coord[Z] << ')';
}

istream &operator>>(istream &i, vector &v)
{
	i >> v.coord[X];
	i >> v.coord[Y];
	i >> v.coord[Z];
	return i;
}

void vector::read(ifstream &f)
{
	coord[X].read(f);
	coord[Y].read(f);
	coord[Z].read(f);
}

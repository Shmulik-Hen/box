#include "attrib.h"

namespace attrib_ns
{

attrib::attrib()
{
}

attrib::~attrib()
{
}

attrib::attrib(unit dx, unit dy, unit dz, unit ox, unit oy, unit oz, unit z) : deg_x{dx}, deg_y{dy}, deg_z{dz}, off_x{ox}, off_y{oy}, off_z{oz}, zoom{z}
{
}

attrib operator+(const attrib &a, const attrib &b)
{
	attrib temp = a;
	temp += b;
	return temp;
}

attrib &attrib::operator+=(const attrib &a)
{
	deg_x += a.deg_x;
	deg_y += a.deg_y;
	deg_z += a.deg_z;
	off_x += a.off_x;
	off_y += a.off_y;
	off_z += a.off_z;
	zoom *= a.zoom;
	return *this;
}

bool attrib::read(ifstream &f)
{
	bool rc = false;
	printf("attrib::read \n");
	rc += deg_x.read(f);
	rc += deg_y.read(f);
	rc += deg_z.read(f);
	rc += off_x.read(f);
	rc += off_y.read(f);
	rc += off_z.read(f);
	rc += zoom.read(f);
	if (!rc) {
		printf("attrib::read error\n");
	}
	return rc;
}

void attrib::print() const
{
	printf("attrib:\n");
	deg_x.print();
	deg_y.print();
	deg_z.print();
	off_x.print();
	off_y.print();
	off_z.print();
	zoom.print();
}

ostream &operator<<(ostream &o, const attrib &a)
{
	return o << '(' << a.deg_x << ',' << a.deg_y << ',' << a.deg_z << ',' << a.off_x << ',' << a.off_y << ',' << a.off_z << ',' << a.zoom << ')';
}

istream &operator>>(istream &i, attrib &a)
{
	i >> a.deg_x;
	i >> a.deg_y;
	i >> a.deg_z;
	i >> a.off_x;
	i >> a.off_y;
	i >> a.off_z;
	i >> a.zoom;
	return i;
}

} // namespace attrib_ns

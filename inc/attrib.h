#ifndef __ATTRIB_H__
#define __ATTRIB_H__

#include <iostream>
#include "common.h"
#include "unit.h"

namespace attrib_ns
{

using std::ifstream;
using std::istream;
using std::ostream;
using unit_ns::unit;

class attrib
{
	unit deg_x{unit_ns::ZERO};
	unit deg_y{unit_ns::ZERO};
	unit deg_z{unit_ns::ZERO};
	unit off_x{unit_ns::ZERO};
	unit off_y{unit_ns::ZERO};
	unit off_z{unit_ns::ZERO};
	unit zoom{unit_ns::ZERO};

public:

	attrib();
	~attrib();
	attrib(unit, unit, unit, unit, unit, unit, unit);
	attrib operator+(const attrib &);
	attrib &operator+=(const attrib &);
	unit get_deg_x() const { return deg_x; };
	unit get_deg_y() const { return deg_y; };
	unit get_deg_z() const { return deg_z; };
	unit get_off_x() const { return off_x; };
	unit get_off_y() const { return off_y; };
	unit get_off_z() const { return off_z; };
	unit get_zoom() const { return zoom; };
	bool read(ifstream &);
	void print() const;
	friend ostream &operator<<(ostream &, const attrib &);
	friend istream &operator>>(istream &, attrib &);
};

} // namespace attrib_ns
#endif //__ATTRIB_H__

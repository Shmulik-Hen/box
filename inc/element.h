#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <fstream>
#include <list>
#include <string>

#include "common.h"
#include "attrib.h"
#include "matrix.h"
#include "polygon.h"
#include "unit.h"

namespace element_ns
{

using attrib_ns::attrib;
using matrix_ns::matrix;
using polygon_ns::polygon;
using std::ifstream;
using std::list;
using std::string;
using unit_ns::unit;

class element : public polygon,
		public matrix
{
public:

	typedef list<element> elem_list;

	element();
	~element();
	element *find_elem(elem_list *, string &) const;
	matrix &get_gen_matrix() { return gen_mat; };
	matrix &get_rot_matrix() { return rot_mat; };
	bool read(poly_list *, ifstream &);
	void print() const;
	void update(const attrib &, matrix &, matrix &);

private:

	typedef elem_list::iterator elem_it;

	string *name{nullptr};
	poly_list *polygons;

	matrix gen_mat;
	matrix rot_mat;
};

} // namespace element_ns
#endif //__ELEMENT_H__

#ifndef __POLYGON_H__
#define __POLYGON_H__

#include <fstream>
#include <list>
#include <string>
#include <queue>

#include "common.h"
#include "my_vector.h"
#include "unit.h"

namespace polygon_ns
{

using my_vector_ns::my_vector;
using std::ifstream;
using std::list;
using std::queue;
using std::string;
using unit_ns::unit;

class polygon : public my_vector
{

public:

	typedef list<polygon> poly_list;
	typedef poly_list::iterator pol_it;

	polygon();
	~polygon();
	const polygon *find_poly(poly_list *, const string &) const;
	polygon *merge_sort();
	polygon *pop();
	void push();
	bool read(ifstream &);
	void print() const;

	int get_force() const { return force; };
	my_vector get_fill() const { return fill; };
	my_vector get_normal() const { return normal; };
	char get_color() const { return color; };
	unit get_depth() const { return depth; };
	void set_fill(my_vector f) { fill = f; };
	void set_normal(my_vector n) { normal = n; };
	void set_color(char c) { color = c; };
	void set_depth(unit d) { depth = d; };

private:

	typedef queue<polygon *> poly_queue;

	string *name{nullptr};
	int force{0};
	char color{0};
	unit depth{unit_ns::ZERO};
	my_vector fill;
	my_vector normal;
	vec_lst *points{nullptr};

	poly_queue *poly_q{nullptr};
	polygon *next{nullptr};

	polygon *merge(polygon *, polygon *);
	my_vector find_fill();
	my_vector find_normal();
};

} // namespace polygon_ns
#endif //__POLYGON_H__

#ifndef __POLYGON_H__
#define __POLYGON_H__

#include <fstream>
#include "vector.h"
#include "list.h"
#include "common.h"

using std::ifstream;
class polygon
{
	NAME name;
	friend class polyelem;

public:

	int force;
	char color;
	vector fill;
	vector normal;
	list<vector> points;

	polygon();
	~polygon();
	vector find_fill();
	vector find_normal();
	unit find_depth();
	friend polygon *find_poly(list<polygon> &, char *);
	friend int poly_comp(const void *);
	void read(ifstream &);
	void print();
};

#endif //__POLYGON_H__

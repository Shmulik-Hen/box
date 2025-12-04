#ifndef _POLYGON_H
#define _POLYGON_H
#include "vector.h"
#include "list.cpp"
#include "types.def"
#include <fstream.h>

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
#endif

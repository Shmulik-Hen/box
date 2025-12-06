#ifndef __POLYELEM_H__
#define __POLYELEM_H__

#include "queue.h"
#include "unit.h"
#include "vector.h"
#include "matrix.h"
#include "polygon.h"

class polyelem : public queue
{
public:

	matrix mat;
	polygon *poly;
	char color;
	unit depth;
	polyelem *next;

	polyelem();
	~polyelem();
	polyelem *merge(polyelem *, polyelem *);
	polyelem *merge_sort();
	void print();
	void show();
};

#endif //__POLYELEM_H__

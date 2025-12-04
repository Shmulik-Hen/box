#include "polyelem.h"
#include <graphics.h>
// #include "graphics.h"

extern vector view;

polyelem::polyelem()
{
	next = NULL;
}

polyelem::~polyelem()
{
}

void polyelem::print()
{
	cout << depth << endl;
	cout << (int)color << endl;
	poly->print();
}

polyelem *merge(polyelem *l1, polyelem *l2)
{
	polyelem *first, *temp, *last = NULL;

	while (l1 && l2) {
		if (l1->depth > l2->depth) {
			temp = l1;
			l1 = l1->next;
		}
		else {
			temp = l2;
			l2 = l2->next;
		}
		if (!last)
			first = temp;
		else
			last->next = temp;
		last = temp;
	}
	if (last)
		if (l1)
			last->next = l1;
		else
			last->next = l2;
	else if (l1)
		first = l1;
	else
		first = l2;
	return first;
}

polyelem *merge_sort()
{
	polyelem *l1, *l2, *l3;
	l1 = (polyelem *)pop();
	l2 = (polyelem *)pop();
	while (l2) {
		l3 = merge(l1, l2);
		l3->push();
		l1 = (polyelem *)pop();
		l2 = (polyelem *)pop();
	}
	return l1;
}

void polyelem::show()
{
	vector *original, transformed, projected;
	int i = 0, scratch_poly[20];

	for (original = poly->points.first(); original; original = poly->points.next()) {
		transformed = mat * *original;
		projected = project(transformed, view);
		scratch_poly[i++] = (int)projected.coord[X] + 160;
		scratch_poly[i++] = -(int)projected.coord[Y] + 100;
	}
	// scratch_poly[i++]=scratch_poly[0];
	// scratch_poly[i++]=scratch_poly[1];
	setcolor(color);
	setfillstyle(1, color);
	fillpoly(i / 2, scratch_poly);
	// drawpoly(i/2,scratch_poly);
}

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "matrix.h"
#include "attrib.h"
#include "polygon.h"
#include "tree.h"
#include "list.h"
#include "polyelem.h"

class element : public treenode
{
	NAME name;
	int dirty_flag;
	int active_flag;
	attrib att;
	matrix gen_mat;
	matrix rot_mat;
	list<polygon> planes;
	friend class ployelem;

public:

	NAME parrent;

	element();
	~element();
	element *find_elem(char *);
	void update(const attrib &);
	friend int elem_comp(const void *);
	friend void update_tree(element *, matrix, matrix);
	friend void printall(element *);
	void read(ifstream &);
	void print();
};

#endif //__ELEMENT_H__

#ifndef _TREE_H
#define _TREE_H
#include <stddef.h>

class treenode
{
protected:

	treenode *next;
	treenode *son;

public:

	treenode() { next = son = NULL; };
	~treenode() {};
	void addnode(treenode *parrent);
	treenode *search(treenode *, int (*comp)(const void *));
};
#endif

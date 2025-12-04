#include "tree.h"

void treenode::addnode(treenode *parrent)
{
	if (!parrent)
		parrent = this;
	else {
		this->next = parrent->son;
		parrent->son = this;
	}
}

treenode *treenode::search(treenode *root, int (*comp)(const void *))
{
	treenode *p;

	if (!root)
		return NULL;
	if (comp(root))
		return root;
	p = search(root->son, comp);
	if (p)
		return p;
	else
		return (search(root->next, comp));
}

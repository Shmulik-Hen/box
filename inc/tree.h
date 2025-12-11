#ifndef __TREE_H__
#define __TREE_H__

#include "stdlib.h"

namespace tree_ns
{

class tree
{
protected:

	tree *sibling{nullptr};
	tree *child{nullptr};

public:

	tree() {};
	~tree() {};
	void addnode(tree *parrent);
	tree *search(tree *, int (*comp)(const void *)) const;
};

} // namespace tree_ns
#endif //__TREE_H__

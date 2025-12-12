#if 0
#include <stdlib.h>
#include "tree.h"

namespace tree_ns
{

tree *tree::addnode(tree *parrent)
{
	if (!parrent)
		parrent = this;
	else {
		this->sibling = parrent->child;
		parrent->child = this;
	}

	return this;
}

tree *tree::search(tree *root, int (*comp)(const void *)) const
{
	tree *p;

	if (!root)
		return nullptr;

	if (comp(root))
		return root;

	p = search(root->child, comp);
	if (p)
		return p;
	else
		return (search(root->sibling, comp));
}

} // namespace tree_ns
#endif // 0

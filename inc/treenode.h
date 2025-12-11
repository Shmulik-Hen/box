#ifndef __TREENODE_H__
#define __TREENODE_H__

#include <fstream>
#include <string>

#include "attrib.h"
#include "element.h"
#include "tree.h"

namespace treenode_ns
{

using attrib_ns::attrib;
using element_ns::element;
using std::ifstream;
using std::string;
using tree_ns::tree;

class treenode : public tree,
		 public element
{

public:

	typedef int (*comp)(const void *);

	treenode();
	~treenode();
	void update_tree(treenode *, matrix &, matrix &);
	bool read(ifstream &);
	void print() const;
	void printall(treenode *) const;
	void show() const;
	bool add_treenode(treenode *);
	void update(const attrib &);
	treenode *find_node(treenode *, string &) const;

private:

	string *name{nullptr};
	string *parrent_name{nullptr};
	string *elem_name{nullptr};
	element *elem{nullptr};
	attrib att;
	int active_flag{0};
	int dirty_flag{0};

	friend int treenode_comp(const void *);
};

} // namespace treenode_ns

#endif //__TREENODE_H__

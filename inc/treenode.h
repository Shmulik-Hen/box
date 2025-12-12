#ifndef __TREENODE_H__
#define __TREENODE_H__

#include <fstream>
#include <string>

#include "attrib.h"
#include "element.h"
// #include "tree.h"

namespace treenode_ns
{

using attrib_ns::attrib;
using element_ns::element;
using std::ifstream;
using std::string;
// using tree_ns::tree;

class treenode : public element
{

public:

	treenode() {};
	~treenode();
	bool read(elem_list *, ifstream &);
	void print() const;
	void printall(treenode *) const;
	void show() const;
	void update(const attrib &);
	void update_tree(treenode *, matrix &, matrix &);
	void add_treenode(treenode **);
	treenode *find_node(treenode *, string &) const;

private:

	treenode *sibling{nullptr};
	treenode *child{nullptr};

	string *name{nullptr};
	string *parrent_name{nullptr};
	string *elem_name{nullptr};
	element *elem{nullptr};
	attrib att;
	int active_flag{0};
	int dirty_flag{0};
};

} // namespace treenode_ns

#endif //__TREENODE_H__

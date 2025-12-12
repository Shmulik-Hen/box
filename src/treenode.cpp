#include <ios>
#include "treenode.h"
#include "utils.h"

namespace treenode_ns
{

using std::ios;

treenode::~treenode()
{
	if (name)
		delete name;

	if (parrent_name)
		delete parrent_name;

	if (elem_name)
		delete elem_name;

	if (elem)
		delete elem;
};

bool treenode::read(elem_list *lst, ifstream &f)
{
	LINE line;
	int finish = 0, len;
	bool rc, ret = true;

	while (!f.eof() && !finish) {
		rc = true;
		while ((!read_word(f, line)) && (!f.eof()))
			;

		if (f.eof())
			break;

		switch (line[1]) {
		case 'n':
			len = read_word(f, line);
			if (len) {
				name = new string(line);
				if (!name) {
					printf("treenode::read allocation error -  name\n");
					fflush(stdout);
					rc = false;
				}
			}
			else {
				printf("treenode::read error name\n");
				fflush(stdout);
				rc = false;
			}
			break;
		case 'p':
			len = read_word(f, line);
			if (len) {
				parrent_name = new string(line);
				if (!parrent_name) {
					printf("treenode::read allocation error -  parrent_name\n");
					fflush(stdout);
					rc = false;
				}
			}
			break;
		case 'e':
			len = read_word(f, line);
			if (len) {
				elem_name = new string(line);
				if (elem_name) {
					elem = find_elem(lst, *elem_name);
				}
				else {
					printf("treenode::read allocation error -  element name\n");
					fflush(stdout);
					rc = false;
				}
			}
			break;
		case 'f':
			len = read_word(f, line);
			if (len) {
				active_flag = atoi(line);
				dirty_flag = 0;
			}
			else {
				printf("treenode::read error flag\n");
				fflush(stdout);
				rc = false;
			}
			break;
		case 'a':
			rc = att.read(f);
			if (!rc) {
				printf("treenode::read error attrib\n");
				fflush(stdout);
				rc = false;
			}
			break;
		default:
			finish = 1;
			f.seekg(-4, ios::cur);
			break;
		}

		if (!rc) {
			printf("treenode::read parsing error\n");
			fflush(stdout);
			ret = false;
		}
	}

	if (!name) {
		name = new string("");
	}

	if (!parrent_name) {
		parrent_name = new string("");
	}

	if (!elem_name) {
		elem_name = new string("");
	}

	if (!elem) {
		elem = new element;
	}

	return ret;
}

void treenode::print() const
{
	printf("  treenode:\n");
	fflush(stdout);
	printf("    name: %s\n", name->c_str());
	fflush(stdout);
	printf("    parrent_name: %s\n", parrent_name->c_str());
	fflush(stdout);
	printf("    active_flag: %d\n", active_flag);
	fflush(stdout);
	printf("    dirty_flag: %d\n", dirty_flag);
	fflush(stdout);
	att.print();
	if (elem)
		elem->print();
}

void treenode::printall(treenode *root) const
{
	if (root) {
		root->print();
		printall((treenode *)root->child);
		printall((treenode *)root->sibling);
	}
}

// void drawpoly(int i, int *arr)
// {
// }

void treenode::show() const
{
	// my_vector *original, transformed, projected;
	// int i = 0, scratch_poly[20];

	// for (original = poly->points.first(); original; original = poly->points.next()) {
	// 	transformed = mat * *original;
	// 	projected = project(transformed, view);
	// 	scratch_poly[i++] = (int)projected.coord[X] + 160;
	// 	scratch_poly[i++] = -(int)projected.coord[Y] + 100;
	// }
	// scratch_poly[i++]=scratch_poly[0];
	// scratch_poly[i++]=scratch_poly[1];
	// setcolor(color);
	// setfillstyle(1, color);
	// fillpoly(i / 2, scratch_poly);
	// drawpoly(i / 2, scratch_poly);
}

void treenode::update(const attrib &a)
{
	att += a;
}

void treenode::update_tree(treenode *node, matrix &p_gen, matrix &p_rot)
{
	if (node) {
		elem->update(att, p_gen, p_rot);
		if (node->active_flag) {
			if (node->child) {
				update_tree(node->child, node->elem->get_gen_matrix(), node->elem->get_rot_matrix());
			}
		}
		if (node->sibling) {
			update_tree(node->sibling, p_gen, p_rot);
		}
	}
}

void treenode::add_treenode(treenode **parrent)
{
	if (!*parrent) {
		*parrent = this;
	}
	else {
		this->sibling = (*parrent)->child;
		(*parrent)->child = this;
	}
}

treenode *treenode::find_node(treenode *root, string &s) const
{
	treenode *p{nullptr};

	if (!root) {
		return nullptr;
	}

	if (*root->name == s) {
		p = root;
	}

	if (!p) {
		p = find_node(root->child, s);
	}

	if (!p) {
		p = find_node(root->sibling, s);
	}

	return p;
}

} // namespace treenode_ns

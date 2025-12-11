#include <fstream>
#include <iostream>
#include <ios>

#include "treenode.h"
#include "utils.h"

namespace treenode_ns
{

using std::ifstream;
using std::ios;

void treenode::update_tree(treenode *node, matrix &p_gen, matrix &p_rot)
{
	if (node) {
		elem->update(att, p_gen, p_rot);
		if (node->active_flag) {
			if (node->child)
				update_tree((treenode *)node->child, node->elem->get_gen_matrix(), node->elem->get_rot_matrix());
		}
		if (node->sibling)
			update_tree((treenode *)node->sibling, p_gen, p_rot);
	}
}

bool treenode::read(ifstream &f)
{
	LINE line;
	string *s;
	int finish = 0, len;
	bool rc = true;

	while (!f.eof() && !finish && rc) {
		rc = true;
		printf("treenode::read\n");
		while ((!read_word(f, line)) && (!f.eof()))
			;

		if (f.eof())
			break;

		switch (line[1]) {
		case 'n':
			printf("treenode::read n:\n");
			len = read_word(f, line);
			if (len) {
				name = new string(line);
				if (name) {
					printf("treenode::read n: %s, %s\n", line, name);
				}
				else {
					printf("treenode::read allocation error -  name\n");
					rc = false;
				}
			}
			else {
				printf("treenode::read error name\n");
				rc - false;
			}
			break;
		case 'p':
			printf("treenode::read f:\n");
			len = read_word(f, line);
			if (len) {
				parrent_name = new string(line);
				if (parrent_name) {
					printf("treenode::read f: %s, %s\n", line, parrent_name);
				}
				else {
					printf("treenode::read allocation error -  parrent_name\n");
					rc = false;
				}
			}
			break;
		case 'e':
			printf("treenode::read e:\n");
			len = read_word(f, line);
			if (len) {
				elem_name = new string(line);
				if (elem_name) {
					printf("treenode::read f: %s, %s\n", line, elem_name);
					elem = (element *)find_elem(elem_lst, *elem_name);
				}
				else {
					printf("treenode::read allocation error -  element name\n");
					rc = false;
				}
			}
			break;
		case 't':
			printf("treenode::read t:\n");
			len = read_word(f, line);
			if (len) {
				active_flag = atoi(line);
				dirty_flag = 0;
				printf("treenode::read t: %s, %d\n", line, active_flag);
			}
			else {
				printf("treenode::read error flag\n");
				rc = false;
			}
			break;
		case 'a':
			printf("treenode::read a:\n");
			rc = att.read(f);
			if (!rc) {
				printf("treenode::read error attrib\n");
			}
			break;
		default:
			printf("treenode::read def: \n");
			finish = 1;
			f.seekg(-4, ios::cur);
			break;
		}
	}

	if (!rc) {
		printf("treenode::read parsing error\n");
		return false;
	}
}

void treenode::print() const
{
	printf("    treenode:\n");
	printf("    name: %s\n", name->c_str());
	printf("    parrent_name %s\n", parrent_name->c_str());
	printf("    active_flag: %d\n", active_flag);
	printf("    dirty_flag: %d\n", dirty_flag);
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

void drawpoly(int i, int *arr)
{
}

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

string cmp_name;

int treenode_comp(const void *p)
{
	treenode *tn = (treenode *)p;
	return (cmp_name == *tn->name);
}

bool treenode::add_treenode(treenode *parrent)
{
	cmp_name = parrent->parrent_name->c_str();
	treenode *p = (treenode *)search(parrent, treenode_comp);
	addnode(p);
}

treenode *treenode::find_node(treenode *root, string &s) const
{
	cmp_name = s;
	treenode *p = (treenode *)search(root, treenode_comp);
}

} // namespace treenode_ns

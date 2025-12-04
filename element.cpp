#include "element.h"
#include <string.h>
#include <stdlib.h>

char *st;
extern list<polygon> poly_list;
extern vector view;
extern vector n_light;
extern matrix UNIT_MAT;

char make_color(char c1, unit c2)
{
	long color = (long)c2;
	if (color >= 1024)
		color = 1023;
	return (char)(((color >> 2) & 0x00f0) | (c1 & 0x0F));
}

element::element()
{
}

element::~element()
{
}

element *element::find_elem(char *s)
{
	element *p;
	cmp fptr = elem_comp;
	st = new char[strlen(s) + 1];
	strcpy(st, s);
	p = (element *)this->search(this, fptr);
	delete[] st;
	return p;
}

int elem_comp(const void *node)
{
	element *p = (element *)node;
	return (!strcmp(st, p->name));
}

void element::update(attrib &a)
{
	att += a;
}

void update_tree(element *node, matrix p_gen, matrix p_rot)
{
	polyelem *pe;
	polygon *poly;
	vector dist, fill, normal;
	unit view_angle, light_angle;

	if (node) {
		if (node->active_flag) {
			prep_gen_mat(node->gen_mat, node->att);
			prep_rot_mat(node->rot_mat, node->att);
			node->gen_mat = p_gen * node->gen_mat;
			node->rot_mat = p_rot * node->rot_mat;
			for (poly = node->planes.first(); poly; poly = node->planes.next()) {
				fill = node->gen_mat * poly->fill;
				normal = node->rot_mat * poly->normal;
				dist = fill - view;
				view_angle = normal * dist;
				if ((view_angle < ZERO) || poly->force) {
					light_angle = normal * n_light;
					if ((light_angle > ZERO) || poly->force) {
						pe = new polyelem;
						pe->depth = dist * dist;
						pe->poly = poly;
						pe->mat = node->gen_mat;
						pe->color = make_color(poly->color, abs(light_angle));
						pe->push();
					}
				}
			}
			if (node->son)
				update_tree((element *)node->son, node->gen_mat, node->rot_mat);
		}
		if (node->next)
			update_tree((element *)node->next, p_gen, p_rot);
	}
}

void element::read(ifstream &f)
{
	LINE line;
	polygon *po;
	int finish = 0;

	while (!f.eof() && !finish) {
		while ((!read_word(f, line)) && (!f.eof()))
			;
		if (f.eof())
			break;
		switch (line[1]) {
		case 'n':
			read_word(f, line);
			strcpy(name, line);
			break;
		case 'f':
			read_word(f, line);
			strcpy(parrent, line);
			break;
		case 't':
			read_word(f, line);
			active_flag = atoi(line);
			dirty_flag = 0;
			break;
		case 'a':
			att.read(f);
			break;
		case 's':
			read_word(f, line);
			po = find_poly(poly_list, line);
			if (po)
				planes.insert(po);
			else
				error("polygon not found in element::read()", line);
			break;
		default:
			finish = 1;
			f.seekg(-4, ios::cur);
			break;
		}
	}
}

void element::print()
{
	polygon *p;

	cout << name << endl;
	cout << parrent << endl;
	cout << att << endl;
	for (p = planes.first(); p; p = planes.next())
		p->print();
}

void printall(element *root)
{
	if (root) {
		root->print();
		printall((element *)root->son);
		printall((element *)root->next);
	}
}

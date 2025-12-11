#include <string.h>
#include <iostream>
#include <ios>
#include <stdio.h>

#include "common.h"
#include "element.h"
#include "polygon.h"
#include "utils.h"

namespace element_ns
{

using my_vector_ns::my_vector;
using std::cout;
using std::endl;
using std::ios;
using namespace unit_ns;

extern my_vector view;
extern my_vector n_light;

element::element()
{
	polygons = new pol_list;
	polygons->clear();
}

element::~element()
{
	if (polygons)
		delete polygons;
}

char make_color(char c1, unit c2)
{
	long color = (long)c2;
	if (color >= 1024)
		color = 1023;
	return (char)(((color >> 2) & 0x00f0) | (c1 & 0x0F));
}

const element *element::find_elem(const elem_list *lst, const string &s)
{
	if (!lst)
		return nullptr;

	for (elem_it it = lst->cbegin(); it != lst->cend(); ++it) {
		const element *e = &*it;
		if (!e)
			return nullptr;

		if (e->name && *e->name == s)
			return e;
	}

	return nullptr;
}

bool element::read(ifstream &f)
{
	LINE line;
	string *s;
	int finish = 0, len;
	bool rc = true;

	while (!f.eof() && !finish && rc) {
		rc = true;
		printf("element::read\n");
		while ((!read_word(f, line)) && (!f.eof()))
			;

		if (f.eof())
			break;

		switch (line[1]) {
		case 'n':
			printf("element::read n:\n");
			len = read_word(f, line);
			if (len) {
				name = new string(line);
				if (name) {
					printf("element::read n: %s, %s\n", line, name);
				}
				else {
					printf("element::read allocation error -  name\n");
					rc = false;
				}
			}
			else {
				printf("element::read error name\n");
				rc = false;
			}
			break;
		case 'p':
			printf("element::read p:\n");
			len = read_word(f, line);
			if (len) {
				printf("element::read p: %s\n", line);
				s = new string(line);
				if (s) {
					const polygon *p = find_poly(polygons, *s);
					if (p) {
						polygons->push_front(*p);
					}
					else {
						printf("element::read find error -  polygon\n");
						rc = false;
					}
				}
				else {
					printf("element::read allocation error -  polygon\n");
					rc = false;
				}
			}
			else {
				printf("element::read error polygon\n");
				rc = false;
			}
		default:
			printf("element::read def: \n");
			finish = 1;
			f.seekg(-4, ios::cur);
			break;
		}

		if (!rc) {
			printf("element::read parsing error\n");
			return false;
		}
	}
}

void element::print() const
{
	for (pol_it it = polygons->begin(); it != polygons->end(); ++it) {
		const polygon *p = &*it;
		if (p)
			p->print();
	}
}

void element::update(const attrib &att, matrix &p_gen, matrix &p_rot)
{
	my_vector dist, fill, normal;
	unit view_angle, light_angle;
	char color;

	gen_mat.prep_gen_mat(att);
	rot_mat.prep_rot_mat(att);
	gen_mat *= p_gen;
	rot_mat *= p_rot;

	for (pol_it it = polygons->begin(); it != polygons->end(); ++it) {
		polygon *p = &*it;
		if (!p)
			return;

		fill = gen_mat * p->get_fill();
		p->set_fill(fill);
		normal = rot_mat * p->get_normal();
		p->set_normal(normal);

		dist = fill - view;
		view_angle = normal * dist;
		if ((view_angle < ZERO) || p->get_force()) {
			light_angle = normal * n_light;
			if ((light_angle > ZERO) || p->get_force()) {
				p->set_depth(dist * dist);
				color = make_color(p->get_color(), abs(light_angle));
				p->set_color(color);
			}
		}
	}
}

} // namespace element_ns

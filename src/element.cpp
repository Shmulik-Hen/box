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
// using namespace unit_ns;

my_vector view(0, 0, -1000000);
my_vector n_light(0, 0, -1024);

element::element()
{
	polygons = new poly_list;
	polygons->clear();
}

element::~element()
{
	if (polygons)
		delete polygons;

	if (name)
		delete name;
}

char make_color(char c1, unit c2)
{
	long color = (long)c2;
	if (color >= 1024)
		color = 1023;
	return (char)(((color >> 2) & 0x00f0) | (c1 & 0x0F));
}

element *element::find_elem(elem_list *lst, string &s) const
{
	for (elem_it it = lst->begin(); it != lst->end(); ++it) {
		element *e = &*it;

		if (e->name && *e->name == s)
			return e;
	}

	return nullptr;
}

bool element::read(poly_list *lst, ifstream &f)
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
					printf("element::read allocation error -  name\n");
					fflush(stdout);
					rc = false;
				}
			}
			else {
				printf("element::read error name\n");
				fflush(stdout);
				rc = false;
			}
			break;
		case 'p':
			len = read_word(f, line);
			if (len) {
				string *s = new string(line);
				if (s) {
					const polygon *p = find_poly(lst, *s);
					if (p) {
						polygons->push_front(*p);
					}
					else {
						printf("element::read find error -  polygon\n");
						fflush(stdout);
						rc = false;
					}
				}
				else {
					printf("element::read allocation error -  polygon\n");
					fflush(stdout);
					rc = false;
				}
			}
			else {
				printf("element::read error polygon\n");
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
			printf("element::read parsing error\n");
			fflush(stdout);
			ret = false;
		}
	}

	if (!name) {
		name = new string("");
	}

	return ret;
}

void element::print() const
{
	if (name) {
		printf("    element:\n      name: %s\n", name->c_str());
		fflush(stdout);
	}

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
		if ((view_angle < unit_ns::ZERO) || p->get_force()) {
			light_angle = normal * n_light;
			if ((light_angle > unit_ns::ZERO) || p->get_force()) {
				p->set_depth(dist * dist);
				color = make_color(p->get_color(), abs(light_angle));
				p->set_color(color);
			}
		}
	}
}

} // namespace element_ns

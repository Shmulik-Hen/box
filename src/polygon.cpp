#include <iostream>
#include <ios>
#include <iterator>
#include <stdlib.h>
#include "polygon.h"
#include "utils.h"

namespace polygon_ns
{

using std::cout;
using std::endl;
using std::ios;

polygon::polygon()
{
	points = new vec_lst;
	points->clear();
	poly_q = new poly_queue;
}

polygon::~polygon()
{
	if (poly_q)
		delete poly_q;

	if (points)
		delete points;

	if (name)
		delete name;
}

const polygon *polygon::find_poly(poly_list *lst, const string &s) const
{
	if (!lst)
		return nullptr;

	for (pol_it it = lst->begin(); it != lst->end(); ++it) {
		const polygon *p = &*it;
		if (!p)
			return nullptr;

		if (p->name && *p->name == s)
			return p;
	}

	return nullptr;
}

polygon *polygon::pop()
{
	polygon *t = poly_q->front();
	poly_q->pop();
	return t;
}

void polygon::push()
{
	poly_q->push(this);
}

polygon *polygon::merge_sort()
{
	polygon *l1, *l2, *l3;
	l1 = pop();
	l2 = pop();
	while (l2) {
		l3 = merge(l1, l2);
		l3->push();
		l1 = pop();
		l2 = pop();
	}

	return l1;
}

bool polygon::read(ifstream &f)
{
	LINE line;
	my_vector *v;
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
					printf("polygon::read allocation error -  polygon\n");
					fflush(stdout);
					rc = false;
				}
			}
			else {
				printf("polygon::read error polygon\n");
				fflush(stdout);
				rc = false;
			}
			break;
		case 'c':
			len = read_word(f, line);
			if (len) {
				color = atoi(line);
			}
			else {
				printf("polygon::read error polygon\n");
				fflush(stdout);
				rc = false;
			}
			break;
		case 'f':
			len = read_word(f, line);
			if (len) {
				force = atoi(line);
			}
			else {
				printf("polygon::read error polygon\n");
				fflush(stdout);
				rc = false;
			}
			break;
		case 'o':
			normal.read(f);
			break;
		case 'v':
			v = new my_vector;
			if (v) {
				if (v->read(f)) {
					points->push_front(*v);
				}
				else {
					printf("polygon::read error polygon\n");
					fflush(stdout);
					rc = false;
				}
			}
			else {
				printf("element::read allocation error -  polygon\n");
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
			printf("polygon::read parsing error\n");
			fflush(stdout);
			ret = false;
		}
	}

	fill = find_fill();
	// normal = find_normal();

	if (!name) {
		name = new string("");
	}

	return ret;
}

void polygon::print() const
{
	printf("      polygon:\n");
	fflush(stdout);
	printf("        name: %s\n", name->c_str());
	fflush(stdout);
	printf("        force: %d\n", (int)force);
	fflush(stdout);
	printf("        color: %c\n", color);
	fflush(stdout);
	printf("        fill:\n");
	fflush(stdout);
	fill.print();
	printf("        normal:\n");
	fflush(stdout);
	normal.print();
	printf("        points:\n");
	fflush(stdout);
	for (vec_it it = points->cbegin(); it != points->cend(); ++it) {
		const my_vector *v = &*it;
		if (v)
			v->print();
	}
}

my_vector polygon::find_fill()
{
	my_vector v;
	long num = 0;

	for (vec_it it = points->cbegin(); it != points->cend(); ++it) {
		v += *it;
		num++;
	}

	unit n(num << 10);
	v.get_coord(coord::X) /= n;
	v.get_coord(coord::Y) /= n;
	v.get_coord(coord::Z) /= n;
	return v;
}

my_vector polygon::find_normal()
{
	vec_it it = points->begin();
	my_vector v1, v2, v;

	if (points->size() >= 2) {
		v1 = *it;
		v2 = *++it;
	}

	v.get_coord(coord::X) = (v1.get_coord(coord::Y) * v2.get_coord(coord::Z) - v1.get_coord(coord::Z) * v2.get_coord(coord::Y));
	v.get_coord(coord::Y) = (v1.get_coord(coord::Z) * v2.get_coord(coord::X) - v1.get_coord(coord::X) * v2.get_coord(coord::Z));
	v.get_coord(coord::Z) = (v1.get_coord(coord::X) * v2.get_coord(coord::Y) - v1.get_coord(coord::Y) * v2.get_coord(coord::X));

	normalize(v);
	return v;
}

polygon *polygon::merge(polygon *l1, polygon *l2)
{
	polygon *first, *temp, *last = nullptr;

	while (l1 && l2) {
		if (l1->depth > l2->depth) {
			temp = l1;
			l1 = l1->next;
		}
		else {
			temp = l2;
			l2 = l2->next;
		}
		if (!last)
			first = temp;
		else
			last->next = temp;
		last = temp;
	}
	if (last)
		if (l1)
			last->next = l1;
		else
			last->next = l2;
	else if (l1)
		first = l1;
	else
		first = l2;

	return first;
}

} // namespace polygon_ns

#include "polygon.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

list<polygon> poly_list;
char *str;

polygon::polygon()
{
}

polygon::~polygon()
{
}

vector polygon::find_fill()
{
	vector *p, temp;
	long num = 0;

	temp -= temp;
	for (p = points.first(); p; p = points.next()) {
		temp += *p;
		num++;
	}
	unit n(num << 10);
	temp.coord[X] /= n;
	temp.coord[Y] /= n;
	temp.coord[Z] /= n;
	return temp;
}

vector polygon::find_normal()
{
	vector *v1, *v2, v;
	v1 = points.first();
	v2 = points.next();
	v.coord[X] = (v1->coord[Y] * v2->coord[Z] - v1->coord[Z] * v2->coord[Y]);
	v.coord[Y] = (v1->coord[Z] * v2->coord[X] - v1->coord[X] * v2->coord[Z]);
	v.coord[Z] = (v1->coord[X] * v2->coord[Y] - v1->coord[Y] * v2->coord[X]);
	normalize(v);
	return v;
}

polygon *find_poly(list<polygon> &lst, char *s)
{
	polygon *p;
	cmp fptr = poly_comp;
	str = new char[strlen(s) + 1];
	strcpy(str, s);
	p = lst.search(fptr);
	delete[] str;
	return p;
}

int poly_comp(const void *link)
{
	polygon *p = (polygon *)link;
	return (!strcmp(str, p->name));
}
/*
int name_comp(const polygon& p,NAME n)
{
 return strcmp(p.name,n);
} */

void polygon::read(ifstream &f)
{
	LINE line;
	vector *vp;
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
		case 'c':
			read_word(f, line);
			color = atoi(line);
			break;
		case 'f':
			read_word(f, line);
			force = atoi(line);
			break;
		case 'o':
			normal.read(f);
			break;
		case 'v':
			vp = new vector;
			vp->read(f);
			points.insert(vp);
			break;
		default:
			finish = 1;
			f.seekg(-4, ios::cur);
			break;
		}
	}
	fill = find_fill();
	// normal=find_normal();
}

void polygon::print()
{
	vector *p;

	cout << name << endl;
	// cout<<color<<endl;
	// cout<<force<<endl;
	cout << "fill:" << fill << endl;
	cout << "normal:" << normal << endl;
	for (p = points.first(); p; p = points.next())
		cout << *p << endl;
}

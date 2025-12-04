#include "element.h"
#include "polyelem.h"
#include <graphics.h>
// #include "graphics.h"
#include <conio.h>
#include <stdlib.h>
#include <dos.h>

extern list<polygon> poly_list;
extern matrix UNIT_MAT;
vector view(0, 0, -1000000);
vector n_light(0, 0, -1024);

typedef char palette[3];
enum rgb
{
	R,
	G,
	B
};
palette pal_16_colors[16];
palette pal_256_colors[256];

void init_palette();
void get_palette(palette buff[], int colors);
void set_palette(palette buff[], int colors);
int huge DetectVGA256();

void main()
{
	char filename[] = "box.dat";
	ifstream f;
	LINE line;
	polygon *poly;
	element *root = NULL, *elem1, *elem2;
	polyelem *pe;
	attrib a(1, 1, 1, 0, 0, 0, 1024);

	f.open(filename, ios::in | ios::nocreate);
	if (!f)
		error("file not found:", filename);
	f.unsetf(ios::skipws);
	while (!f.eof()) {
		while ((!read_word(f, line)) && (!f.eof()))
			;
		switch (line[1]) {
		case '#':
			read_remark(f);
		case 'p':
			poly = new polygon;
			poly->read(f);
			poly_list.insert(poly);
			break;
		case 'e':
			elem1 = new element;
			elem1->read(f);
			if (!root)
				root = elem1;
			else {
				elem2 = root->find_elem(elem1->parrent);
				if (elem2)
					elem1->addnode(elem2);
				else
					error("element not found in tree::addnode()", elem1->parrent);
			}
			break;
		}
	}
	f.close();

	int Gd = DETECT, Gm;
	installuserdriver("SVGA256", DetectVGA256);
	initgraph(&Gd, &Gm, "");
	init_palette();
	setviewport(1, 1, 319, 199, 1);

	elem1 = root->find_elem("world");
	elem2 = root->find_elem("box");
	elem2->update(attrib(-32, 0, 0, 0, 0, 0, 2048));
	while (!kbhit()) {
		elem2->update(attrib(0, 3, 7, 0, 0, 0, 1024));
		update_tree(root, UNIT_MAT, UNIT_MAT);
		pe = merge_sort();
		clearviewport();
		while (pe) {
			polyelem *tmp = pe;
			pe = pe->next;
			tmp->show();
			delete tmp;
		}
		delay(17);
	}
	closegraph();
}

void init_palette()
{
	int i, j;

	get_palette(pal_16_colors, 16);
	for (i = 0; i < 16; i++)
		for (j = 0; j < 16; j++) {
			pal_256_colors[j * 16 + i][R] = (char)(((int)(pal_16_colors[i][R]) * j) / 15);
			pal_256_colors[j * 16 + i][G] = (char)(((int)(pal_16_colors[i][G]) * j) / 15);
			pal_256_colors[j * 16 + i][B] = (char)(((int)(pal_16_colors[i][B]) * j) / 15);
		}
	set_palette(pal_256_colors, 256);
}

void get_palette(palette buff[], int colors)
{
	struct REGPACK reg;

	reg.r_ax = 0x1017;
	reg.r_bx = 0;
	reg.r_cx = colors;
	reg.r_dx = FP_OFF(buff);
	reg.r_es = FP_SEG(buff);
	intr(0x10, &reg);
}

void set_palette(palette buff[], int colors)
{
	struct REGPACK reg;

	reg.r_ax = 0x1012;
	reg.r_bx = 0;
	reg.r_cx = colors;
	reg.r_dx = FP_OFF(buff);
	reg.r_es = FP_SEG(buff);
	intr(0x10, &reg);
}

int huge DetectVGA256()
{
	return 0;
	/*
		 int Vid;

		 printf("Which video mode would you like to use? \n");
		 printf("  0) 320x200x256\n");
		 printf("  1) 640x400x256\n");
		 printf("  2) 640x480x256\n");
		 printf("  3) 800x600x256\n");
		 printf("  4) 1024x768x256\n\n>");
		 scanf("%d",&Vid);
		 return Vid;
	*/
}

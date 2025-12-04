#ifndef _UTILS_H
#define _UTILS_H
#include <fstream.h>
#include "types.def"

void error(const char *s1, const char *s2 = "");
int read_word(ifstream &, char *);
void read_remark(ifstream &);
#endif

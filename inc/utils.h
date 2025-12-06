#ifndef __UTILS_H__
#define __UTILS_H__

#include <fstream>
#include "common.h"

using std::ifstream;

void error(const char *s1, const char *s2 = "");
int read_word(ifstream &, char *);
void read_remark(ifstream &);
#endif

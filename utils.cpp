#include "utils.h"
#include <process.h>
// #include "graphics.h"
#include <graphics.h>

void error(const char *s1, const char *s2)
{
	closegraph();
	cerr << s1 << ' ' << s2;
	exit(0);
}

int read_word(ifstream &f, char *word)
{
	word[0] = '\0';
	int finish = 0;
	int length = 0;
	char c;

	while ((!finish) && (!f.eof()) && (length < MAX_LINE)) {
		f >> c;
		if ((c <= 32) || (c == ',') || (c == '(') || (c == ')') || (c == '{') || (c == '}'))
			finish = 1;
		else
			word[length++] = c;
	}
	word[length] = '\0';
	return length;
}

void read_remark(ifstream &f)
{
}

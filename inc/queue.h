#ifndef __QUEUE_H__
#define __QUEUE_H__
#include <stddef.h>

class queue
{
	static queue *head;
	static queue *tail;
	queue *nextq;

public:

	queue();
	~queue();
	void push();
	friend queue *pop();
};
#endif

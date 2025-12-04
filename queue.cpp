#include "queue.h"

queue *queue::head = NULL;
queue *queue::tail = NULL;

queue::queue()
{
	nextq = NULL;
}

queue::~queue()
{
}

void queue::push()
{
	if (!tail)
		tail = head = this;
	else {
		tail->nextq = this;
		tail = tail->nextq;
	}
}

queue *pop()
{
	if (!queue::head)
		return NULL;
	queue *temp = queue::head;
	queue::head = queue::head->nextq;
	temp->nextq = NULL;
	if (!queue::head)
		queue::tail = NULL;
	return temp;
}

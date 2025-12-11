#if 0
#include <stdlib.h>
#include "queue.h"

namespace queue_ns
{

queue *queue::head = nullptr;
queue *queue::tail = nullptr;

queue::queue()
{
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

queue *queue::pop()
{
	if (!queue::head)
		return nullptr;
	queue *temp = queue::head;
	queue::head = queue::head->nextq;
	temp->nextq = nullptr;
	if (!queue::head)
		queue::tail = nullptr;
	return temp;
}

} // namespace queue_ns
#endif // 0

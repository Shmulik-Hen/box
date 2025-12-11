#if 0
#ifndef __QUEUE_H__
#define __QUEUE_H__

namespace queue_ns
{
class queue
{
	static queue *head;
	static queue *tail;
	queue *nextq{nullptr};
public:

	queue();
	~queue();
	void push();
	queue *pop();
};

} // namespace queue_ns
#endif //__QUEUE_H__
#endif // 0

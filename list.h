#ifndef _LIST_H
#define _LIST_H

template <class T> class list
{
	typedef struct tag_link
	{
		T *item;
		tag_link *next;
	} link;
	link *head;
	link *current;

public:

	list();
	~list();
	void insert(T *);
	T *first();
	T *next();
	T *search(int (*comp)(const void *));
};
#endif

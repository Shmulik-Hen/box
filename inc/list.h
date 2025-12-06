#ifndef __LIST_H__
#define __LIST_H__

#include "utils.h"
template <class T> class list
{
	struct link
	{
		T *item;
		link *next;
	};

	link *head;
	link *current;

public:

	list() : head{NULL}, current{NULL} {};

	~list()
	{
		link *temp;
		while (head) {
			temp = head;
			head = head->next;
			if (temp->item)
				delete temp->item;
			delete temp;
		}
	};

	void insert(T *newitem)
	{
		link *temp = new link;
		if (!temp)
			error("not enough memory in list::insert()");
		temp->item = newitem;
		temp->next = NULL;
		if (!head)
			head = temp;
		else {
			temp->next = head;
			head = temp;
		}
	};

	T *first()
	{
		current = head;
		if (current)
			return current->item;
		else
			return NULL;
	};

	T *next()
	{
		current = current->next;
		if (current)
			return current->item;
		else
			return NULL;
	};

	T *search(int (*comp)(const void *))
	{
		link *temp = head;
		while (temp) {
			if (comp(temp->item))
				return temp->item;
			temp = temp->next;
		}
		return NULL;
	};
};

#endif //__LIST_H__

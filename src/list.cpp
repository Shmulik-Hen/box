#if 0
#include <stddef.h>
#include "list.h"
#include "utils.h"

template <class T> list<T>::list()
{
	current = head = nullptr;
}

template <class T> list<T>::~list()
{
	link *temp;
	while (head) {
		temp = head;
		head = head->next;
		if (temp->item)
			delete temp->item;
		delete temp;
	}
}

template <class T> void list<T>::insert(T *newitem)
{
	link *temp = new link;
	if (!temp)
		error("not enough memory in list::insert()");
	temp->item = newitem;
	temp->next = nullptr;
	if (!head)
		head = temp;
	else {
		temp->next = head;
		head = temp;
	}
}

template <class T> T *list<T>::first()
{
	current = head;
	if (current)
		return current->item;
	else
		return nullptr;
}

template <class T> T *list<T>::next()
{
	current = current->next;
	if (current)
		return current->item;
	else
		return nullptr;
}
/*
template<class T>
T* list<T>::find(const T& t,int(*comp)(const void*,const void*))
{
 link* temp=head;
 while(temp)
   {
    if(!comp(temp->item,&t))
      return temp->item;
    temp=temp->next;
   }
 return nullptr;
} */

template <class T> T *list<T>::search(int (*comp)(const void *))
{
	link *temp = head;
	while (temp) {
		if (comp(temp->item))
			return temp->item;
		temp = temp->next;
	}
	return nullptr;
}

/*
template<class T>
int list<T>::empty()
{
 return (head==nullptr);
}*/
#endif // 0

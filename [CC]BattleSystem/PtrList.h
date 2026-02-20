#ifndef _PTRLIST_STUB_H
#define _PTRLIST_STUB_H

// CC模块客户端编译时的PtrList存根

#include "../[CC]Header/CommonStruct.h"

template<class TYPE>
class PtrList
{
public:
	PtrList() : m_head(NULL), m_tail(NULL), m_count(0) {}
	virtual ~PtrList() {}

	TYPE* GetHead() { return m_head; }
	TYPE* GetTail() { return m_tail; }
	TYPE* GetNext(TYPE* ptr) { return ptr ? ptr->next : NULL; }
	int GetCount() { return m_count; }

	void AddNode(TYPE* node)
	{
		if (!node) return;
		node->next = NULL;
		if (m_tail) m_tail->next = node;
		m_tail = node;
		if (!m_head) m_head = node;
		m_count++;
	}

	void RemoveNode(TYPE* node)
	{
		// 简化实现
		m_count--;
	}

	void RemoveAll()
	{
		m_head = m_tail = NULL;
		m_count = 0;
	}

private:
	TYPE* m_head;
	TYPE* m_tail;
	int m_count;
};

#endif

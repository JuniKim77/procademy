#pragma once
#include "operatorNewOverload.h"

template <typename T>
class myList
{
public:
	struct Node
	{
		T data;
		Node* prev;
		Node* next;
	};

	class iterator
	{
	public:
		friend class myList;
		iterator(Node* node = nullptr)
			: _node(node)
		{
			//���ڷ� ���� Node �����͸� ����
		}

		iterator operator ++(int)
		{
			//���� ��带 ���� ���� �̵�
			iterator cur(_node);
			_node = _node->next;
			return cur;
		}

		iterator& operator++()
		{
			_node = _node->next;
			return *this;
		}

		iterator operator --(int)
		{
			iterator cur(_node);
			_node = _node->prev;
			return cur;
		}

		iterator& operator--()
		{
			_node = _node->prev;
			return *this;
		}

		T& operator *()
		{
			//���� ����� �����͸� ����
			return _node->data;
		}
		bool operator ==(const iterator& other)
		{
			return _node == other._node;
		}
		bool operator !=(const iterator& other)
		{
			return _node != other._node;
		}

	private:
		Node* _node;
	};

public:
	myList();
	~myList();

	iterator begin()
	{
		//ù��° ������ ��带 ����Ű�� ���ͷ����� ����
		return iterator(_head.next);
	}
	iterator end()
	{
		/*Tail ��带 ����Ű��(�����Ͱ� ���� ��¥ ���� �� ���) ���ͷ����͸� ����
			�Ǵ� ������ ������ �� �ִ� ���ͷ����͸� ����*/
		return iterator(&_tail);
	}

	void push_front(T data);
	void push_back(T data);
	T pop_front();
	T pop_back();
	void clear();
	int size() { return _size; };
	bool empty() { return _size == 0; };
	/*	- ���ͷ������� �� ��带 ����.
		- �׸��� ���� ����� ���� ��带 ī��Ű�� ���ͷ����� ����*/
	iterator erase(iterator iter)
	{
		if (_size == 0)
		{
			return end();
		}

		Node* cur = iter._node;
		cur->prev->next = cur->next;
		cur->next->prev = cur->prev;
		Node* next = cur->next;
		delete cur;
		--_size;

		return iterator(next);
	}

	void remove(T Data);

private:
	int _size = 0;
	Node _head;
	Node _tail;
};

template<typename T>
inline myList<T>::myList()
{
	_head.data = 0;
	_head.prev = nullptr;
	_head.next = &_tail;
	_tail.data = 0;
	_tail.next = nullptr;
	_tail.prev = &_head;
}

template<typename T>
inline myList<T>::~myList()
{
}

template<typename T>
inline void myList<T>::push_front(T data)
{
	Node* node = new Node;
	node->data = data;
	node->prev = &_head;
	node->next = _head.next;
	_head.next->prev = node;
	_head.next = node;
	++_size;
}

template<typename T>
inline void myList<T>::push_back(T data)
{
	Node* node = new Node;
	node->data = data;
	node->prev = _tail.prev;
	node->next = &_tail;
	_tail.prev->next = node;
	_tail.prev = node;
	++_size;
}

template<typename T>
inline T myList<T>::pop_front()
{
	if (_size == 0)
	{
		return _head.data;
	}

	Node* cur = _head.next;
	T ret = cur->data;
	_head.next = cur->next;
	cur->next->prev = &_head;
	delete cur;
	--_size;

	return ret;
}

template<typename T>
inline T myList<T>::pop_back()
{
	if (_size == 0)
	{
		return _tail.data;
	}

	Node* cur = _tail.prev;
	T ret = cur->data;
	_tail.prev = cur->prev;
	cur->prev->next = &_tail;
	delete cur;
	--_size;

	return ret;
}

template<typename T>
inline void myList<T>::clear()
{
	Node* cur = _head.next;
	for (int i = 0; i < _size; ++i)
	{
		Node* next = cur->next;
		delete cur;
		cur = next;
	}
	_head.next = &_tail;
	_tail.prev = &_head;

	_size = 0;
}

template<typename T>
inline void myList<T>::remove(T Data)
{
	int num = _size;
	Node* cur = _head.next;

	for (int i = 0; i < num; ++i)
	{
		Node* next = cur->next;

		if (cur->data = Data)
		{
			cur->prev->next = next;
			next->prev = cur->prev;
			delete cur;
			--_size;
		}

		cur = next;
	}
}

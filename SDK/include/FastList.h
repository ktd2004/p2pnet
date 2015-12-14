/* Copyright 2014 by Lee yong jun
 * All rights reserved
 *
 * Distribute freely, except: don't remove my name from the source or
 * documentation (don't take credit for my work), mark your changes (don't
 * get me blamed for your possible bugs), don't alter or remove this
 * notice.  May be sold if buildable source is provided to buyer.  No
 * warrantee of any kind, express or implied, is included with this
 * software; use at your own risk, responsibility for damages (if any) to
 * anyone resulting from the use of this software rests entirely with the
 * user.
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Lee yong jun          iamhere01@naver.com
 */
#pragma once

namespace util
{
	template <class T1>	class list
	{
	public:
		struct node
		{
			T1*		m_prev;
			T1*		m_next;
		};
		struct fncompare
		{
			virtual bool operator == (T1 *) = 0;
		};

	protected:
		node			_Head;
		node			_Tail;
		unsigned int	count;

	public :
		list() { clear(); }
		virtual ~list() {}

		// iterator class
		class iterator
		{
		public :
			iterator() : m_node(NULL) {}
			iterator( T1 *elt ) : m_node( (node*)elt ) {}
			iterator( node *elt ) : m_node( elt ) {}
			~iterator() {}

			T1 * operator* () const { return (T1*)m_node; }
			T1 * operator-> () const { return (T1*)m_node; }

			iterator & operator= (const iterator& _Right)
			{
				m_node = _Right.m_node;
				return *this;
			}

			iterator & operator++ ()
			{
				m_node = m_node->m_next;
				return *this;
			}

			iterator operator++ (int)
			{
				iterator _tmp = *this;
				++*this;
				return _tmp;
			}

			iterator & operator-- ()
			{
				m_node = m_node->m_prev;
				return *this;
			}

			iterator operator-- (int)
			{
				iterator _tmp = *this;
				--*this;
				return _tmp;
			}

			bool operator== (const iterator& _Right)
			{ return m_node == _Right.m_node; }
			bool operator!= (const iterator& _Right)
			{ return (!(*this == _Right)); }

		private:
			node * m_node;
		};

		inline iterator begin()
		{
			return ( iterator(_Head.m_next) );
		}

		inline iterator end()
		{
			return ( iterator(&_Tail) );
		}

		inline bool empty()
		{
			return _Head.m_next == (T1*)&_Tail;
		}

		inline void clear()
		{
			_Head.m_prev = NULL;
			_Head.m_next = (T1*)&_Tail;
			_Tail.m_prev = (T1*)&_Head;
			_Tail.m_next = NULL;
			count = 0;
		}

		inline T1 * front()
		{
			if ( empty() ) return NULL;
			return _Head.m_next;
		}

		inline T1 * back()
		{
			if ( empty() ) return NULL;
			return _Tail.m_prev;
		}

		inline void push_front(T1 * _Val)
		{
			_Val->m_prev = (T1*)&_Head;
			_Val->m_next = _Head.m_next;
			_Head.m_next->m_prev = _Val;
			_Head.m_next = _Val;
			count++;
		}

		inline void	pop_front()
		{
			if ( empty() ) return;
			T1 *p = _Head.m_next;
			_Head.m_next = p->m_next;
			_Head.m_next->m_prev =(T1*)&_Head;
			p->m_prev = NULL;
			p->m_next = NULL;
			count--;
		}

		inline void push_back(T1 * _Val)
		{
			_Val->m_next = (T1*)&_Tail;
			_Val->m_prev = _Tail.m_prev;
			_Tail.m_prev->m_next = _Val;
			_Tail.m_prev = _Val;
			count++;
		}

		inline void pop_back()
		{
			if ( empty() ) return;
			T1 *p = _Tail.m_prev;
			p->m_prev->m_next = (T1*)&_Tail;
			_Tail.m_prev = p->m_prev;
			p->m_next = NULL;
			p->m_prev = NULL;
			count--;
		}

		inline iterator	insert(iterator& _Where, T1 * _Val)
		{
			iterator it(*_Where);
			_Val->m_prev = _Where->m_prev;
			_Val->m_next = (*_Where);
			_Where->m_prev->m_next = _Val;
			_Where->m_prev = _Val;
			count++;
			return ( iterator((*it)));
		}


		inline iterator	erase(iterator& _Where)
		{
			iterator it((*_Where)->m_next);
			_Where->m_next->m_prev = _Where->m_prev;
			_Where->m_prev->m_next = _Where->m_next;
			_Where->m_next = NULL;
			_Where->m_prev = NULL;
			count--;
			return ( it );
		}

		inline void remove(T1 * _Val)
		{
			if ( empty() || !_Val->m_prev || !_Val->m_next ) return;
			erase( iterator(_Val) );
		}

		inline void swap(util::list<T1> &rlist)
		{
			util::list<T1> target;
			target.merge( rlist );
			rlist.merge( *this );
			merge( target );
		}

		inline void merge(util::list<T1> &rlist)
		{
			if ( rlist.empty() ) return;

			T1 *s = rlist._Head.m_next;
			T1 *e = rlist._Tail.m_prev;
			T1 *p = NULL;

			p = _Tail.m_prev;
			_Tail.m_prev = e;
			e->m_next = (T1*)&_Tail;
			p->m_next = s;
			s->m_prev = p;
			count += rlist.count;
			rlist.clear();
		}

		inline iterator find( iterator itS, iterator itE, T1 * Val)
		{
			while ( itS != end() && itS != itE )
			{
				if ( (*itS) == Val )
					return itS;
				itS++;
			}
			return end();
		}

		inline iterator find_if( iterator itS, iterator itE, fncompare &cmp_func )
		{
			while ( itS != end() && itS != itE )
			{
				if ( cmp_func == (*itS) )
					return itS;
				itS++;
			}
			return end();
		}

		inline iterator remove_if( iterator itS, iterator itE, fncompare &cmp_func )
		{
			while ( itS != end() && itS != itE )
			{
				if ( cmp_func == (*itS) )
				{
					return erase( itS );
				}
				itS++;
			}
			return end();
		}

		inline unsigned int size()
		{ return count; }
	};
}


#define listnode(x)		util::list<x>::node
#define listcompare(x)	util::list<x>::fncompare
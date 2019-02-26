#ifndef _HN_SMART_POINTER_
#define _HN_SMART_POINTER_

#include "HNReference.h"

namespace HN
{
	template <typename T, bool = std::is_class<T>::value && std::is_base_of<CReferenceObject, T>::value>
	class CSmartPointer
	{
	public:
		explicit CSmartPointer()
			: m_ptr( NULL )
		{
		}

		explicit CSmartPointer( T * ptr )
			: m_ptr( ptr )
		{
			if( m_ptr )
				m_ptr->AddRef();
		}

		CSmartPointer( CSmartPointer & other )
			: m_ptr( other.m_ptr )
		{
			if( m_ptr )
				m_ptr->AddRef();
		}

		~CSmartPointer()
		{
			if( m_ptr )
				m_ptr->Release();
		}
	public:
		T * operator ->()
		{
			return m_ptr;
		}
		T & operator *()
		{
			return *m_ptr;
		}
		operator T *()
		{
			return m_ptr;
		}
	public:
		bool empty()
		{
			return NULL == m_ptr;
		}
	protected:
		T *		m_ptr;
	};

	template <typename T>
	class CSmartPointer<T, false>
		: public CReferenceShell<T>
	{
	public:
		explicit CSmartPointer()
			: CReferenceShell( NULL )
		{
		}

		explicit CSmartPointer( T * ptr )
			: CReferenceShell( ptr )
		{
		}

		CSmartPointer( CReferenceShell & rsObj )
			: CReferenceShell( rsObj )
		{
		}

		CSmartPointer( CSmartPointer & other )
			: CReferenceShell( other )
		{
		}
	};
}

#endif // _HN_SMART_POINTER_
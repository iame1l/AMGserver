#pragma once

#include <type_traits>
#include <intrin.h>

namespace HN
{
	class CReferenceObject
	{
	public:
		CReferenceObject()
			: m_lRef(1)
		{
			printf("new %x\n", this);
		}
		virtual ~CReferenceObject()
		{
			printf("delete %x\n", this);
		}
	public:
		long AddRef()
		{
			return _InterlockedIncrement( &m_lRef );
		}

		long Release()
		{
			long lResult = _InterlockedDecrement( &m_lRef );

			if( 0 == lResult )
			{
				delete this;
			}

			return lResult;
		}
	private:
		long volatile m_lRef;
	};

	template <typename T>
	class CReferenceShell
	{
		class CRef
			: public CReferenceObject
		{
		public:
			CRef( T * ptr )
				: m_ptr(ptr)
			{
			}
			virtual ~CRef()
			{
				if( m_ptr )
					delete m_ptr;
			}
		public:
			T * m_ptr;
		};
	public:
		explicit CReferenceShell(T * ptr)
			: m_pRef( ptr ? new CRef( ptr ) : NULL )
		{
		}

		CReferenceShell( CReferenceShell & other )
			: m_pRef(other.m_pRef)
		{
			if( m_pRef )
				m_pRef->AddRef();
		}

		~CReferenceShell()
		{
			if( m_pRef )
				m_pRef->Release();
		}
	public:
		T * operator ->()
		{
			return m_pRef->m_ptr;
		}

		T & operator *()
		{
			return *(m_pRef->m_ptr);
		}

		operator T *()
		{
			return m_pRef ? m_pRef->m_ptr : NULL;
		}

		bool empty()
		{
			return NULL == m_pRef || NULL == m_pRef->m_ptr;
		}
	public:
		CReferenceShell & operator = (CReferenceShell & other)
		{
			if( other.m_pRef )
				other.m_pRef->AddRef();

			if( m_pRef )
				m_pRef->Release();

			m_pRef = other.m_pRef;

			return *this;
		}
	private:
		CRef *	m_pRef;
	};


}
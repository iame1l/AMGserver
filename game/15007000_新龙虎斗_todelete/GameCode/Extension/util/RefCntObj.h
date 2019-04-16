#ifndef REF_CNT_OBJ_H
#define REF_CNT_OBJ_H
#include <iostream>

template<typename T>
class RefCntObjImpl
{
    int m_RefCnt;
    T *m_Obj;
public:
    RefCntObjImpl(T *obj):m_RefCnt(1), m_Obj(obj){}
    void Reserve(){ ++m_RefCnt;}
    int Release(){ return --m_RefCnt;}
    T *GetObj(){ return m_Obj;}
};

template<typename T>
class RefCntObjRecycler
{
public:
    void Recycle(T *obj)
    {
        if(obj != NULL) delete obj;
    }
};

template<typename T, typename Recycler = RefCntObjRecycler<T> >
class RefCntObj
{
    Recycler m_Recycler;
    RefCntObjImpl<T> *m_Impl;

    void Release()
    {
        if(m_Impl->Release() == 0)
        {
            m_Recycler.Recycle(m_Impl->GetObj());
            delete m_Impl;
        }
    }

    void Copy(const RefCntObj &other)
    {
        m_Recycler = other.m_Recycler;
        m_Impl = other.m_Impl;
        m_Impl->Reserve();
    }

public:

    RefCntObj(T *obj)
    {
        m_Impl = new RefCntObjImpl<T>(obj);
    }

    RefCntObj(const RefCntObj &other)
    {
        Copy(other);
    }

    RefCntObj &operator = (const RefCntObj &other)
    {
        if(m_Impl != other.m_Impl)
        {
            Release();
            Copy(other);
        }
        return *this;
    }

    T *operator -> ()
    {
        return m_Impl->GetObj();
    }

    const T *operator -> () const
    {
        return m_Impl->GetObj();
    }

    T &operator * ()
    {
        return *m_Impl->GetObj();
    }

    T &operator * () const
    {
        return *m_Impl->GetObj();
    }

    T *GetObj(){ return m_Impl->GetObj();}

    const T *GetObj() const { return m_Impl->GetObj();}

	bool IsValid() const
	{
		return m_Impl->GetObj() != NULL;
	}

    virtual ~RefCntObj()
    {
        Release();
    }
};

#endif

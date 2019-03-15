#include "ExtensionObject.h"
#include <cstddef>

namespace ExtObj
{
    void RemoveExtension(Extension *ext)
    {
        delete ext;
    }

    void Destroy(ExtensionObject *obj)
    {
        delete obj;
    }
}

ExtensionObject::~ExtensionObject()
{
    for(ExtensionList::iterator itr = m_Extensions.begin();
        itr != m_Extensions.end();
        ++itr)
    {
        ExtObj::RemoveExtension(itr->second);
    }
    m_Extensions.clear();
}

Extension *ExtensionObject::GetExtension(ExtObjIdType id)
{
    Extension *ret = NULL;
    ExtensionList::iterator itr = m_Extensions.find(id);
    if(itr != m_Extensions.end()) ret = itr->second;
    return ret;
}

Extension *ExtensionObject::AddExtension(ExtObjIdType id, Extension *ext)
{
    ExtensionList::iterator itr = m_Extensions.find(id);
    if(itr != m_Extensions.end())
    {
        // 如果已有，删除
        ExtObj::RemoveExtension(itr->second);
    }

    m_Extensions[id] = ext;
    OnAddExtension(ext);

    return ext;
}

void ExtensionObject::RemoveExtension(ExtObjIdType id)
{
    ExtensionList::iterator itr = m_Extensions.find(id);
    if(itr != m_Extensions.end())
    {
        OnRemoveExtension(itr->second);
        ExtObj::RemoveExtension(itr->second);
        m_Extensions.erase(itr);
    }
}

ExtensionList &ExtensionObject::GetExtensions()
{
    return m_Extensions;
}

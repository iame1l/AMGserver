#ifndef EXTENSION_ID_H
#define EXTENSION_ID_H

#include <map>
#include <typeinfo>

typedef void* ExtObjIdType;

template<typename T>
class ExtObjID
{
public:
    ExtObjIdType ID;
    ExtObjID()
    {
        ID = (ExtObjIdType)(typeid(*this).name());
    }
};

#endif

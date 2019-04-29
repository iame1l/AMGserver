#ifndef EXTENSION_OBJECT_H
#define EXTENSION_OBJECT_H

#include <map>
#include "ExtensionID.h"

class ExtensionObject;
// 扩展
class Extension
{
public:
	Extension() : m_pExt(NULL) {}
    virtual ~Extension(){}
public:
	ExtensionObject * m_pExt;
};

typedef std::map<ExtObjIdType, Extension*> ExtensionList;

// 扩展对象
class ExtensionObject
{
protected:
    ExtensionList m_Extensions;
public:
    virtual ~ExtensionObject();
    Extension *GetExtension(ExtObjIdType id);
    Extension *AddExtension(ExtObjIdType id, Extension *);
    void RemoveExtension(ExtObjIdType id);
    ExtensionList &GetExtensions();
    virtual void OnAddExtension(Extension *ext){}
    virtual void OnRemoveExtension(Extension *ext){}
};

namespace ExtObj
{
    // 获取扩展
    template<typename T>
    T *GetExtension(ExtensionObject *obj, ExtObjIdType id)
    {
        return (T*)obj->GetExtension(id);
    }

    // 增加扩展
    template<typename T>
    T *AddExtension(ExtensionObject *obj, ExtObjIdType id)
    {
        return (T*)obj->AddExtension(id, new T());
    }

    // 创建扩展对象
    template<typename T>
    T *Create()
    {
        return new T();
    }

    // 销毁扩展对象
    void Destroy(ExtensionObject *obj);

    // 删除扩展
    void RemoveExtension(Extension *ext);
}

// 创建扩展对象： @extType 扩展类型
#define CreateExtObj(extType) ExtObj::Create<extType>()

// 销毁扩展对象： @obj 扩展对象
#define DestroyExtObj(obj) ExtObj::Destroy(obj)

// 获取扩展：@obj 扩展对象  @extType 扩展类型
#define ObjGetExt(obj, extType) ExtObj::GetExtension<extType>(obj, ExtObjID<extType>().ID)

// 增加扩展：@obj 扩展对象  @extType 扩展类型
#define ObjAddExt(obj, extType) ExtObj::AddExtension<extType>(obj, ExtObjID<extType>().ID)

// 删除扩展：@obj 扩展对象  @extType 扩展类型
#define ObjRmExt(obj, extType) obj->RemoveExtension(extType)

// 获取扩展：@obj 扩展对象  @extType 扩展类型  @varName 变量名
#define OBJ_GET_EXT(obj, extType, varName)\
    extType *varName = ObjGetExt(obj, extType);\
    varName = (varName ? varName : ObjAddExt(obj, extType));

#endif

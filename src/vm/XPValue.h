#ifndef __XPVvalue_h
#define __XPVvalue_h
#include <string>

enum class XPValueType
{
    NUMBER,
    OBJECT
};

enum class ObjectType
{
    STRING,
    CODE
};

struct Object
{
    Object(ObjectType type) : type(type) {}
    ObjectType type;
};

struct StringObject : public Object
{
    StringObject(const std::string &str) : Object(ObjectType::STRING), string(str) {}
    std::string string;
};

struct XPValue
{
    XPValueType type;
    union
    {
        double number;
        Object *object;
    };
};

struct CodeObject : public Object
{
    CodeObject(const std::string &name) : Object(ObjectType::CODE), name(name) {}

    std::string name;
    std::vector<uint8_t> code;
    std::vector<XPValue> constants;
};

#define NUMBER(value) ((XPValue){XPValueType::NUMBER, .number = value})
#define ALLOC_STRING(value) \
    ((XPValue){XPValueType::OBJECT, .object = (Object *)new StringObject(value)})
#define ALLOC_CODE(name) \
    ((XPValue){XPValueType::OBJECT, .object = (Object *)new CodeObject(name)})

#define AS_NUMBER(xPValue) ((double)(xPValue).number)
#define AS_OBJECT(xPValue) ((Object *)(xPValue).object)

#define AS_STRING(xPValue) ((StringObject *)(xPValue).object)
#define AS_CODE(xPValue) ((CodeObject *)(xPValue).object)
#define AS_CPPSTRING(xPValue) (AS_STRING(xPValue)->string)

#define IS_NUMBER(xpValue) ((xpValue).type == XPValueType::NUMBER)
#define IS_OBJECT(xpValue) ((xpValue).type == XPValueType::OBJECT)

#define IS_OBJECT_TYPE(xpValue, objectType) \
    (IS_OBJECT(xpValue) && AS_OBJECT(xpValue)->type == objectType)

#define IS_STRING(xpValue) IS_OBJECT_TYPE(xpValue, ObjectType::STRING)
#define IS_CODE(xpValue) IS_OBJECT_TYPE(xpValue, ObjectType::CODE)

#endif
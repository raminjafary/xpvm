#ifndef __XPVvalue_h
#define __XPVvalue_h

#include <string>
#include <vector>
#include "../Logger.h"

enum class XPValueType
{
    NUMBER,
    BOOLEAN,
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
        bool boolean;
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
#define BOOLEAN(value) ((XPValue){XPValueType::BOOLEAN, .boolean = value})
#define ALLOC_STRING(value) \
    ((XPValue){XPValueType::OBJECT, .object = (Object *)new StringObject(value)})
#define ALLOC_CODE(name) \
    ((XPValue){XPValueType::OBJECT, .object = (Object *)new CodeObject(name)})

#define AS_NUMBER(xPValue) ((double)(xPValue).number)
#define AS_OBJECT(xPValue) ((Object *)(xPValue).object)
#define AS_BOOLEAN(xPValue) ((bool)(xPValue).boolean)

#define AS_STRING(xPValue) ((StringObject *)(xPValue).object)
#define AS_CODE(xPValue) ((CodeObject *)(xPValue).object)
#define AS_CPPSTRING(xPValue) (AS_STRING(xPValue)->string)

#define IS_NUMBER(xpValue) ((xpValue).type == XPValueType::NUMBER)
#define IS_OBJECT(xpValue) ((xpValue).type == XPValueType::OBJECT)
#define IS_BOOLEAN(xpValue) ((xpValue).type == XPValueType::BOOLEAN)

#define IS_OBJECT_TYPE(xpValue, objectType) \
    (IS_OBJECT(xpValue) && AS_OBJECT(xpValue)->type == objectType)

#define IS_STRING(xpValue) IS_OBJECT_TYPE(xpValue, ObjectType::STRING)
#define IS_CODE(xpValue) IS_OBJECT_TYPE(xpValue, ObjectType::CODE)

std::string xpValueToTypeString(const XPValue &value)
{
    if (IS_NUMBER(value))
    {
        return "NUMBER";
    }
    else if (IS_BOOLEAN(value))
    {
        return "BOOLEAN";
    }
    else if (IS_STRING(value))
    {
        return "STRING";
    }
    else if (IS_CODE(value))
    {
        return "CODE";
    }
    else
    {
        DIE << "xpValueToTypeString unknown type: " << (int)value.type;
    }
    return "";
}

std::string xpValueToConstantString(const XPValue &value)
{
    std::stringstream ss;

    if (IS_NUMBER(value))
    {
        ss << value.number;
    }
    else if (IS_BOOLEAN(value))
    {
        ss << (value.boolean == true ? "true" : "false");
    }
    else if (IS_STRING(value))
    {
        ss << '"' << AS_CPPSTRING(value) << '"';
    }
    else if (IS_CODE(value))
    {
        auto code = AS_CODE(value);
        ss << "code " << code << ": " << code->name;
    }
    else
    {
        DIE << "xpValueToConstantString unknown value: " << (int)value.type;
    }

    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const XPValue &xpValue)
{
    return os << "XPValue ("
              << xpValueToTypeString(xpValue)
              << "): " << xpValueToConstantString(xpValue);
}

#endif
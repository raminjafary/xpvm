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
    CODE,
    NATIVE
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

using NativeFunction = std::function<void()>;

struct NativeObject : public Object
{
    NativeObject(NativeFunction function, const std::string &name, size_t arity)
        : Object(ObjectType::NATIVE),
          function(function),
          name(name),
          arity(arity) {}

    NativeFunction function;
    std::string name;
    size_t arity;
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

struct LocalVar
{
    std::string name;
    size_t scoleLevel;
};

struct CodeObject : public Object
{
    CodeObject(const std::string &name) : Object(ObjectType::CODE), name(name) {}

    std::string name;
    std::vector<uint8_t> code;
    std::vector<XPValue> constants;

    size_t scopeLevel = 0;
    std::vector<LocalVar> locals;

    void addLocal(const std::string &name)
    {
        locals.push_back({name, scopeLevel});
    }

    int getlocalIndex(const std::string &name)
    {
        if (locals.size() > 0)
        {
            for (auto i = (int)locals.size() - 1; i >= 0; i--)
            {
                if (locals[i].name == name)
                {
                    return i;
                }
            }
        }

        return -1;
    }
};

#define NUMBER(value) ((XPValue){XPValueType::NUMBER, .number = value})
#define BOOLEAN(value) ((XPValue){XPValueType::BOOLEAN, .boolean = value})
#define ALLOC_STRING(value) \
    ((XPValue){XPValueType::OBJECT, .object = (Object *)new StringObject(value)})
#define ALLOC_CODE(name) \
    ((XPValue){XPValueType::OBJECT, .object = (Object *)new CodeObject(name)})
#define ALLOC_NATIVE(fn, name, arity) \
    ((XPValue){XPValueType::OBJECT, .object = (Object *)new NativeObject(fn, name, arity)})

#define AS_NUMBER(xPValue) ((double)(xPValue).number)
#define AS_OBJECT(xPValue) ((Object *)(xPValue).object)
#define AS_BOOLEAN(xPValue) ((bool)(xPValue).boolean)
#define AS_NATIVE(xPValue) ((NativeObject *)(xPValue).object)

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
#define IS_NATIVE(xpValue) IS_OBJECT_TYPE(xpValue, ObjectType::NATIVE)

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
    else if (IS_NATIVE(value))
    {
        return "NATIVE";
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
    else if (IS_NATIVE(value))
    {
        auto fn = AS_NATIVE(value);
        ss << fn->name << "/" << fn->arity;
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
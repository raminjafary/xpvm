#ifndef __XPVvalue_h
#define __XPVvalue_h

enum class XPValueType
{
    NUMBER,
};

struct XPValue
{
    XPValueType type;
    union
    {
        double number;
    };
};

#define NUMBER(value) ((XPValue){XPValueType::NUMBER, .number = value})

#define AS_NUMBER(xPValue) ((double)(xPValue).number)

#endif
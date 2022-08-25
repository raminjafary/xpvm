#ifndef __XP_Compiler_h
#define __XP_Compiler_h

#include "../vm/XPValue.h"
#include "../bytecode/OpCode.h"

class XPCompiler
{
public:
    XPCompiler() {}

    CodeObject *compile(const Exp &exp)
    {
        co = AS_CODE(ALLOC_CODE("main"));

        gen(exp);
        emit(OP_HALT);
        return co;
    }

    void gen(const Exp &exp)
    {
        switch (exp.type)
        {
        case ExpType::NUMBER:
            emit(OP_CONST);
            emit(numericConstIdx(exp.number));
            break;
        case ExpType::STRING:
            emit(OP_CONST);
            emit(stringConstIdx(exp.string));
            break;

        case ExpType::SYMBOL:
            break;
        case ExpType::LIST:
            break;
        default:
            break;
        }
    }

    size_t numericConstIdx(double value)
    {
        for (auto i = 0; i < co->constants.size(); i++)
        {
            if (!IS_NUMBER(co->constants[i]))
            {
                continue;
            }

            if (AS_NUMBER(co->constants[i]) == value)
            {
                return i;
            }
        }
        co->constants.push_back(NUMBER(value));
        return co->constants.size() - 1;
    }

    size_t stringConstIdx(const std::string &value)
    {
        for (auto i = 0; i < co->constants.size(); i++)
        {
            if (!IS_NUMBER(co->constants[i]))
            {
                continue;
            }

            if (AS_CPPSTRING(co->constants[i]) == value)
            {
                return i;
            }
        }
        co->constants.push_back(ALLOC_STRING(value));
        return co->constants.size() - 1;
    }

    void emit(uint8_t code)
    {
        co->code.push_back(code);
    }

    CodeObject *co;
};

#endif
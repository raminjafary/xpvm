#ifndef __XP_Compiler_h
#define __XP_Compiler_h

#include "../vm/XPValue.h"
#include "../bytecode/OpCode.h"

#define ALLOC_CONST(tester, convertor, allocator, value) \
    do                                                   \
    {                                                    \
        for (auto i = 0; i < co->constants.size(); i++)  \
        {                                                \
            if (!tester(co->constants[i]))               \
            {                                            \
                continue;                                \
            }                                            \
            if (convertor(co->constants[i]) == value)    \
            {                                            \
                return i;                                \
            }                                            \
        }                                                \
        co->constants.push_back(allocator(value));       \
    } while (false)

#define GEN_BINARY_OP(op) \
    do                    \
    {                     \
        gen(exp.list[1]); \
        gen(exp.list[2]); \
        emit(op);         \
    } while (false)

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
        {
            auto tag = exp.list[0];

            if (tag.type == ExpType::SYMBOL)
            {
                auto op = tag.string;

                if (op == "+")
                {
                    GEN_BINARY_OP(OP_ADD);
                }
                else if (op == "-")
                {
                    GEN_BINARY_OP(OP_SUB);
                }
                else if (op == "/")
                {
                    GEN_BINARY_OP(OP_DIV);
                }
                else if (op == "*")
                {
                    GEN_BINARY_OP(OP_MUL);
                }
            }
            break;
        }

        default:
            break;
        }
    }

    size_t numericConstIdx(double value)
    {
        ALLOC_CONST(IS_NUMBER, AS_NUMBER, NUMBER, value);
        return co->constants.size() - 1;
    }

    size_t stringConstIdx(const std::string &value)
    {
        ALLOC_CONST(IS_STRING, AS_CPPSTRING, ALLOC_STRING, value);
        return co->constants.size() - 1;
    }

    void emit(uint8_t code)
    {
        co->code.push_back(code);
    }

    CodeObject *co;
};

#endif
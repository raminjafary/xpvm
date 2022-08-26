#ifndef __xp_h
#define __xp_h

#include <iostream>
#include <array>
#include <string>
#include <vector>

#include "../Logger.h"
#include "../bytecode/OpCode.h"
#include "../parser/XPParser.h"
#include "../compiler/XPCompiler.h"
#include "XPValue.h"

using syntax::XPParser;

#define READ_BYTE() *ip++

#define STACK_LIMIT 512

#define GET_CONST() co->constants[READ_BYTE()]

#define BINARY_OP(op)                \
    do                               \
    {                                \
        auto op2 = AS_NUMBER(pop()); \
        auto op1 = AS_NUMBER(pop()); \
        push(NUMBER(op1 op op2));    \
    } while (false)

#define COMPARE_VALUES(op, op1, op2) \
    do                               \
    {                                \
        bool res;                    \
        switch (op)                  \
        {                            \
        case 0:                      \
            res = op1 < op2;         \
            break;                   \
        case 1:                      \
            res = op1 > op2;         \
            break;                   \
        case 2:                      \
            res = op1 == op2;        \
            break;                   \
        case 3:                      \
            res = op1 >= op2;        \
            break;                   \
        case 4:                      \
            res = op1 <= op2;        \
            break;                   \
        case 5:                      \
            res = op1 != op2;        \
            break;                   \
        }                            \
        push(BOOLEAN(res));          \
    } while (false)

class XPVM
{
public:
    XPVM() : parser(std::make_unique<XPParser>()),
             compiler(std::make_unique<XPCompiler>()) {}

    void push(const XPValue &value)
    {
        if ((size_t)(sp - stack.begin()) == STACK_LIMIT)
        {
            DIE << "stack overflow!";
        }
        *sp = value;
        sp++;
    }

    XPValue pop()
    {
        if (sp == stack.begin())
        {
            DIE << "empty stack!";
        }
        --sp;
        return *sp;
    }

    XPValue exec(const std::string &program)

    {
        auto ast = parser->parse(program);

        co = compiler->compile(ast);

        ip = &co->code[0];

        sp = &stack[0];

        return eval();
    }

    XPValue eval()
    {
        for (;;)
        {
            auto opcode = READ_BYTE();

            switch (opcode)
            {
            case OP_HALT:
                return pop();

            case OP_CONST:
                // auto constIndex = READ_BYTE();
                // auto constant = constants[constIndex];
                push(GET_CONST());
                break;

            case OP_ADD:
            {
                auto op2 = pop();
                auto op1 = pop();

                if (IS_NUMBER(op1) && IS_NUMBER(op2))
                {
                    push(NUMBER(AS_NUMBER(op1) + AS_NUMBER(op2)));
                }
                else if (IS_STRING(op1) && IS_STRING(op2))
                {
                    auto s1 = AS_CPPSTRING(op1);
                    auto s2 = AS_CPPSTRING(op2);
                    push(ALLOC_STRING(s1 + s2));
                }
                break;
            }

            case OP_DIV:
                BINARY_OP(/);
                break;
            case OP_MUL:
                BINARY_OP(*);
                break;
            case OP_SUB:
                BINARY_OP(-);
                break;
            case OP_COMPARE:
            {
                auto op = READ_BYTE();

                auto op2 = pop();
                auto op1 = pop();

                if (IS_NUMBER(op2) && IS_NUMBER(op1))
                {
                    COMPARE_VALUES(op, AS_NUMBER(op1), AS_NUMBER(op2));
                }
                else if (IS_STRING(op1) && IS_STRING(op2))
                {
                    COMPARE_VALUES(op, AS_CPPSTRING(op1), AS_CPPSTRING(op2));
                }
                break;
            }

            default:
                DIE << "Unknown opcode: " << std::hex << int(opcode);
            }
        }
    }

    uint8_t *ip;

    XPValue *sp;

    std::unique_ptr<XPParser> parser;
    std::unique_ptr<XPCompiler> compiler;

    std::array<XPValue, STACK_LIMIT> stack;

    CodeObject *co;
};

#endif
#ifndef __xp_h
#define __xp_h

#include <iostream>
#include <array>
#include <string>
#include <vector>

#include "./XPValue.h"
#include "../Logger.h"
#include "../bytecode/OpCode.h"

#define READ_BYTE() *ip++

#define STACK_LIMIT 512

#define GET_CONST() constants[READ_BYTE()]

#define BINARY_OP(op)                \
    do                               \
    {                                \
        auto op2 = AS_NUMBER(pop()); \
        auto op1 = AS_NUMBER(pop()); \
        push(NUMBER(op1 op op2));    \
    } while (false)

class XPVM
{
public:
    void xpVM() {}

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

        constants.push_back(NUMBER(6));
        constants.push_back(NUMBER(5));
        constants.push_back(NUMBER(10));

        code = {OP_CONST, 0, OP_CONST, 1, OP_SUB, OP_CONST, 2, OP_ADD, OP_HALT};

        ip = &code[0];

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
                BINARY_OP(+);
                break;
            case OP_DIV:
                BINARY_OP(/);
                break;
            case OP_MUL:
                BINARY_OP(*);
                break;
            case OP_SUB:
                BINARY_OP(-);
                break;
            default:
                DIE << "Unknown opcode: " << std::hex << int(opcode);
            }
        }
    }

    uint8_t *ip;

    XPValue *sp;

    std::array<XPValue, STACK_LIMIT> stack;

    std::vector<XPValue> constants;

    std::vector<uint8_t> code;
};

#endif
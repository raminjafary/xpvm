#ifndef __OpCode_h
#define __OpCode_h

#include "../Logger.h"

#define OP_HALT 0x00

#define OP_CONST 0x01

#define OP_ADD 0x02
#define OP_SUB 0x03
#define OP_MUL 0x04
#define OP_DIV 0x05

#define OP_COMPARE 0x06

#define OP_JMP_IF_FALSE 0x07
#define OP_JMP 0x08

#define OP_GET_GLOBAL 0x09
#define OP_SET_GLOBAL 0x10

#define OP_POP 0x11

#define OP_GET_LOCAL 0x12
#define OP_SET_LOCAL 0x13

#define OP_SCOPE_EXIT 0x14

#define OP_CALL 0x15

#define OP_RETURN 0x16

#define OP_GET_CELL 0x17

#define OP_SET_CELL 0x18

#define OP_LOAD_CELL 0x19

#define OP_MAKE_FUNCTION 0x20

#define OP_STR(opcode) \
    case OP_##opcode:  \
        return #opcode

std::string opcodeToString(uint8_t opcode)
{
    switch (opcode)
    {
        OP_STR(HALT);
        OP_STR(CONST);
        OP_STR(ADD);
        OP_STR(SUB);
        OP_STR(MUL);
        OP_STR(COMPARE);
        OP_STR(JMP_IF_FALSE);
        OP_STR(JMP);
        OP_STR(GET_GLOBAL);
        OP_STR(SET_GLOBAL);
        OP_STR(POP);
        OP_STR(GET_LOCAL);
        OP_STR(SET_LOCAL);
        OP_STR(SCOPE_EXIT);
        OP_STR(CALL);
        OP_STR(RETURN);
        OP_STR(GET_CELL);
        OP_STR(SET_CELL);
        OP_STR(LOAD_CELL);
        OP_STR(MAKE_FUNCTION);
    default:
        DIE << "opcodeToString: unknown opcode" << std::hex << (int)opcode;
    }

    return "Unknown";
}
#endif
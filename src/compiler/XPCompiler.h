#ifndef __XP_Compiler_h
#define __XP_Compiler_h

#include <string>
#include <map>
#include "../vm/XPValue.h"
#include "../bytecode/OpCode.h"
#include "../disassembler/disassembler.h"

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
    XPCompiler() : disassembler(std::make_unique<Disassembler>()) {}

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
            if (exp.string == "true" || exp.string == "false")
            {
                emit(OP_CONST);
                emit(booleanConstIdx(exp.string == "true" ? true : false));
            }
            else
            {
            }
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
                else if (compareOps.count(op) != 0)
                {
                    gen(exp.list[1]);
                    gen(exp.list[2]);
                    emit(OP_COMPARE);
                    emit(compareOps[op]);
                }
                else if (op == "if")
                {
                    gen(exp.list[1]);

                    emit(OP_JMP_IF_FALSE);

                    emit(0);
                    emit(0);

                    auto elseJmpAddress = getOffset() - 2;

                    gen(exp.list[2]);
                    emit(OP_JMP);

                    emit(0);
                    emit(0);

                    auto endAddress = getOffset() - 2;

                    auto elseBranchAddress = getOffset();
                    patchJmpAddress(elseJmpAddress, elseBranchAddress);

                    if (exp.list.size() == 4)
                    {
                        gen(exp.list[3]);
                    }

                    auto endBranchAddress = getOffset();
                    patchJmpAddress(endAddress, endBranchAddress);
                }
            }
            break;
        }

        default:
            break;
        }
    }

    void disassembleByteCode()
    {
        disassembler->disassemble(co);
    }

private:
    std::unique_ptr<Disassembler> disassembler;
    size_t getOffset()
    {
        return co->code.size();
    }

    void writeByteAtOffset(size_t offset, uint8_t value)
    {
        co->code[offset] = value;
    }

    void patchJmpAddress(size_t offset, uint16_t value)
    {
        writeByteAtOffset(offset, (value >> 8) & 0xff);
        writeByteAtOffset(offset + 1, value & 0xff);
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

    size_t booleanConstIdx(bool value)
    {
        ALLOC_CONST(IS_BOOLEAN, AS_BOOLEAN, BOOLEAN, value);
        return co->constants.size() - 1;
    }

    void emit(uint8_t code)
    {
        co->code.push_back(code);
    }

    CodeObject *co;

    static std::map<std::string, uint8_t> compareOps;
};

std::map<std::string, uint8_t> XPCompiler::compareOps = {
    {"<", 0},
    {">", 1},
    {"==", 2},
    {">=", 3},
    {"<=", 4},
    {"!=", 5},
};

#endif
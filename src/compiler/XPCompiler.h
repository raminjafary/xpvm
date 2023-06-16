#ifndef __XP_Compiler_h
#define __XP_Compiler_h

#include <string>
#include <map>
#include "../vm/XPValue.h"
#include "../bytecode/OpCode.h"
#include "../vm/globalVar.h"
#include "../disassembler/disassembler.h"
#include "../scope/scope.h"

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
        co->addConstant(allocator(value));               \
    } while (false)

#define GEN_BINARY_OP(op) \
    do                    \
    {                     \
        gen(exp.list[1]); \
        gen(exp.list[2]); \
        emit(op);         \
    } while (false)

#define FUNCTION_CALL(exp)                         \
    do                                             \
    {                                              \
        gen(exp.list[0]);                          \
        for (auto i = 1; i < exp.list.size(); i++) \
        {                                          \
            gen(exp.list[i]);                      \
        }                                          \
        emit(OP_CALL);                             \
        emit(exp.list.size() - 1);                 \
    } while (false)

class XPCompiler
{
public:
    XPCompiler(std::shared_ptr<Global> global) : global(global),
                                                 disassembler(std::make_unique<Disassembler>(global)) {}

    void compile(const Exp &exp)
    {
        co = AS_CODE(createCodeObjectValue("main"));
        main = AS_FUNCTION(ALLOC_FUNCTION(co));
        constantObjects_.insert((Traceable *)main);

        analyze(exp, nullptr);

        gen(exp);
        emit(OP_HALT);
    }

    void analyze(const Exp &exp, std::shared_ptr<Scope> scope)
    {

        if (exp.type == ExpType::SYMBOL)
        {

            if (exp.string == "true" || exp.string == "false")
            {
            }
            else
            {
                scope->maybePromote(exp.string);
            }
        }
        else if (exp.type == ExpType::LIST)
        {
            auto tag = exp.list[0];

            if (tag.type == ExpType::SYMBOL)
            {
                auto op = tag.string;

                if (op == "begin")
                {
                    auto newScope = std::make_shared<Scope>(
                        scope == nullptr ? ScopeType::GLOBAL : ScopeType::BLOCK, scope);

                    scopeInfo_[&exp] = newScope;

                    for (auto i = 1; i < exp.list.size(); i++)
                    {
                        analyze(exp.list[i], newScope);
                    }
                }

                else if (op == "var")
                {
                    scope->addLocal(exp.list[1].string);
                    analyze(exp.list[2], scope);
                }
                else if (op == "def")
                {
                    auto fnName = exp.list[1].string;

                    scope->addLocal(fnName);

                    auto newScope = std::make_shared<Scope>(ScopeType::FUNCTION, scope);

                    scopeInfo_[&exp] = newScope;

                    auto arity = exp.list[2].list.size();

                    for (auto i = 0; i < arity; i++)
                    {
                        newScope->addLocal(exp.list[2].list[i].string);
                    }

                    analyze(exp.list[3], newScope);
                }
                else if (op == "lambda")
                {
                    auto newScope = std::make_shared<Scope>(ScopeType::FUNCTION, scope);

                    scopeInfo_[&exp] = newScope;

                    auto arity = exp.list[1].list.size();

                    for (auto i = 0; i < arity; i++)
                    {
                        newScope->addLocal(exp.list[1].list[i].string);
                    }

                    analyze(exp.list[2], newScope);
                }
                else
                {
                    for (auto i = 1; i < exp.list.size(); i++)
                    {
                        analyze(exp.list[i], scope);
                    }
                }
            }
            else
            {
                for (auto i = 0; i < exp.list.size(); i++)
                {
                    analyze(exp.list[i], scope);
                }
            }
        }
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
                auto varName = exp.string;

                auto opCodeGetter = scopeStack_.top()->getNameGetter(varName);
                emit(opCodeGetter);

                if (opCodeGetter == OP_GET_LOCAL)
                {
                    emit(co->getlocalIndex(varName));
                }
                else if (opCodeGetter == OP_GET_CELL)
                {
                    emit(co->getCellIndex(varName));
                }
                else
                {

                    if (!global->exists(varName))
                    {
                        DIE << "[Compiler]: Refrence error: " << varName;
                    }

                    emit(global->getGlobalIndex(varName));
                }
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

                else if (op == "while")
                {
                    auto loopStartAddress = getOffset();

                    gen(exp.list[1]);

                    emit(OP_JMP_IF_FALSE);

                    emit(0);
                    emit(0);

                    auto loopEndJmpAddress = getOffset() - 2;

                    gen(exp.list[2]);
                    emit(OP_JMP);

                    emit(0);
                    emit(0);

                    auto endAddress = getOffset() - 2;

                    patchJmpAddress(endAddress, loopStartAddress);

                    auto loopEndAddress = getOffset() + 1;
                    patchJmpAddress(loopEndJmpAddress, loopEndAddress);
                }

                else if (op == "var")
                {
                    auto varName = exp.list[1].string;

                    auto opCodeSetter = scopeStack_.top()->getNameSetter(varName);

                    if (isLambda(exp.list[2]))
                    {
                        compileFunction(
                            exp.list[2],
                            varName,
                            exp.list[2].list[1],
                            exp.list[2].list[2]);
                    }
                    else
                    {
                        gen(exp.list[2]);
                    }

                    if (opCodeSetter == OP_SET_GLOBAL)
                    {

                        global->define(varName);
                        emit(OP_SET_GLOBAL);
                        emit(global->getGlobalIndex(varName));
                    }
                    else if (opCodeSetter == OP_SET_CELL)
                    {
                        co->cellNames.push_back(varName);
                        emit(OP_SET_CELL);
                        emit(co->cellNames.size() - 1);
                        emit(OP_POP);
                    }
                    else
                    {
                        co->addLocal(varName);
                        // emit(OP_SET_LOCAL);
                        // emit(co->getlocalIndex(varName));
                    }
                }

                else if (op == "set")
                {
                    auto varName = exp.list[1].string;

                    auto opCodeSetter = scopeStack_.top()->getNameSetter(varName);

                    gen(exp.list[2]);

                    if (opCodeSetter == OP_SET_LOCAL)
                    {
                        emit(OP_SET_LOCAL);
                        emit(co->getlocalIndex(varName));
                    }
                    else if (opCodeSetter == OP_SET_CELL)
                    {
                        emit(OP_SET_CELL);
                        emit(co->getCellIndex(varName));
                    }
                    else
                    {
                        auto globalIndex = global->getGlobalIndex(varName);

                        if (globalIndex == -1)
                        {
                            DIE << "Refrence error: " << varName << " is not defined!";
                        }
                        emit(OP_SET_GLOBAL);
                        emit(globalIndex);
                    }
                }
                else if (op == "begin")
                {
                    scopeStack_.push(scopeInfo_.at(&exp));

                    blockEnter();

                    for (auto i = 1; i < exp.list.size(); i++)
                    {
                        bool isLast = i == exp.list.size() - 1;

                        // auto isLocalDeclaration = isDeclaration(exp.list[i]) && !isGlobalScope();

                        auto isDecl = isDeclaration(exp.list[i]);

                        gen(exp.list[i]);

                        if (!isLast && !isDecl)
                        {
                            emit(OP_POP);
                        }
                    }

                    blockExit();
                    scopeStack_.pop();
                }
                else if (op == "def")
                {
                    auto fnName = exp.list[1].string;

                    compileFunction(
                        exp,
                        fnName,
                        exp.list[2],
                        exp.list[3]);

                    if (isGlobalScope())
                    {

                        global->define(fnName);
                        emit(OP_SET_GLOBAL);
                        emit(global->getGlobalIndex(fnName));
                    }
                    else
                    {
                        co->addLocal(fnName);
                        // emit(OP_SET_LOCAL);
                        // emit(co->getlocalIndex(fnName));
                    }
                }
                else if (op == "lambda")
                {
                    compileFunction(
                        exp,
                        "lambda",
                        exp.list[1],
                        exp.list[2]);
                }
                else
                {
                    FUNCTION_CALL(exp);
                }
            }
            else
            {
                FUNCTION_CALL(exp);
            }
            break;
        }

        default:
            break;
        }
    }

    void compileFunction(const Exp &exp, const std::string &fnName,
                         const Exp &params, const Exp &body)
    {
        auto scopeInfo = scopeInfo_.at(&exp);
        scopeStack_.push(scopeInfo);

        auto arity = params.list.size();

        auto prevCo = co;

        auto coValue = createCodeObjectValue(fnName, arity);
        co = AS_CODE(coValue);

        co->freeCount = scopeInfo->free.size();

        co->cellNames.reserve(scopeInfo->free.size() + scopeInfo->cells.size());

        co->cellNames.insert(co->cellNames.end(), scopeInfo->free.begin(), scopeInfo->free.end());

        co->cellNames.insert(co->cellNames.end(), scopeInfo->cells.begin(), scopeInfo->cells.end());

        prevCo->addConstant(coValue);
        co->addLocal(fnName);

        for (auto i = 0; i < arity; i++)
        {
            auto argName = params.list[i].string;
            co->addLocal(argName);

            auto cellIndex = co->getCellIndex(argName);
            if (cellIndex != -1)
            {
                emit(OP_SET_CELL);
                emit(cellIndex);
            }
        }

        gen(body);

        if (!isBlock(body))
        {
            emit(OP_SCOPE_EXIT);
            emit(arity + 1);
        }

        emit(OP_RETURN);

        if (scopeInfo->free.size() == 0)
        {

            auto fn = ALLOC_FUNCTION(co);
            constantObjects_.insert((Traceable *)AS_OBJECT(fn));

            co = prevCo;

            co->addConstant(fn);

            emit(OP_CONST);
            emit(co->constants.size() - 1);
        }
        else
        {
            co = prevCo;

            for (const auto &freeVar : scopeInfo->free)
            {
                emit(OP_LOAD_CELL);
                emit(prevCo->getCellIndex(freeVar));
            }

            emit(OP_CONST);
            emit(co->constants.size() - 1);
            emit(OP_MAKE_FUNCTION);

            emit(scopeInfo->free.size());
        }

        scopeStack_.pop();
    }

    FunctionObject *getMainFunction()
    {
        return main;
    }

    XPValue createCodeObjectValue(const std::string &name, size_t arity = 0)
    {
        auto coValue = ALLOC_CODE(name, arity);
        auto co = AS_CODE(coValue);
        codeObjects_.push_back(co);
        constantObjects_.insert((Traceable *)co);
        return coValue;
    }

    void disassembleByteCode()
    {
        for (auto &co_ : codeObjects_)
        {
            disassembler->disassemble(co_);
        }
    }

private:
    size_t getOffset()
    {
        return co->code.size();
    }

    bool isDeclaration(const Exp &exp)
    {
        return isVarDeclaration(exp);
    }

    bool isVarDeclaration(const Exp &exp)
    {
        return isTaggedList(exp, "var");
    }

    bool isTaggedList(const Exp &exp, const std::string &tag)
    {
        return exp.type == ExpType::LIST && exp.list[0].type == ExpType::SYMBOL && exp.list[0].string == tag;
    }

    bool isBlock(const Exp &exp)
    {
        return isTaggedList(exp, "begin");
    }

    bool isLambda(const Exp &exp)
    {
        return isTaggedList(exp, "lambda");
    }

    bool isGlobalScope()
    {
        return co->name == "main" && co->scopeLevel == 1;
    }

    bool isFunctionBody()
    {
        return co->name != "main" && co->scopeLevel == 1;
    }

    std::set<Traceable *> &getConstantObjects()
    {
        return constantObjects_;
    }

    void blockEnter()
    {
        co->scopeLevel++;
    }

    void blockExit()
    {
        auto varCounts = getVarCountOnScopeExit();

        if (varCounts > 0 || co->arity > 0)
        {
            emit(OP_SCOPE_EXIT);

            if (isFunctionBody())
            {
                varCounts += co->arity + 1;
            }

            emit(varCounts);
        }
        co->scopeLevel--;
    }

    size_t getVarCountOnScopeExit()
    {
        auto varCount = 0;

        if (co->locals.size() > 0)
        {
            while (co->locals.back().scoleLevel == co->scopeLevel)
            {
                co->locals.pop_back();
                varCount++;
            }
        }
        return varCount;
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

    std::map<const Exp *, std::shared_ptr<Scope>> scopeInfo_;

    std::stack<std::shared_ptr<Scope>> scopeStack_;

    CodeObject *co;

    FunctionObject *main;

    std::vector<CodeObject *> codeObjects_;

    std::set<Traceable *> constantObjects_;

    std::shared_ptr<Global> global;

    std::unique_ptr<Disassembler> disassembler;

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
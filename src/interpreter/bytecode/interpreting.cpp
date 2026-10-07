#include "interpreting.h"

#include <iostream>

#include "compiler.h"
#include "decompile.h"

using namespace Bytecode;

#define READ_BYTE() (*ip++)
#define READ_SHORT() (ip += 2, (u16)((ip[-2] << 8) | ip[-1]))
#define READ_OP() (static_cast<Operation>(READ_BYTE()))

//#define BYTECODE_SAFETY

#define FRAME_COUNT (256)
#define STACK_SIZE (FRAME_COUNT * UINT8_COUNT)

struct CallFrame {
    Value::Value* slots;
    u8* returnip;
    Value::Function* func;
};

class VM {
    u8* ip;
    int sp;
    u16 fp;

#ifndef BYTECODE_SAFETY
    Value::Value* constants;
#endif

    Program& program;

    Value::Value *globals;

    CallFrame callStack[FRAME_COUNT] {};
    Value::Value stack[STACK_SIZE] {};

    Value::Value pop() {
#ifdef BYTECODE_SAFETY
        if (sp == 0) {
            std::cerr << "Value stack underflow" << std::endl;
            exit(-1);
        }
#endif
        return stack[--sp];
    }
    
    [[nodiscard]] Value::Value peek(u16 dist) const {
        return stack[sp - dist - 1];
    }
    
    void push(Value::Value value) {
#ifdef BYTECODE_SAFETY
        if (sp == STACK_SIZE) {
            std::cerr << "Value stack overflow" << std::endl;
            exit(-1);
        }
#endif
        stack[sp++] = value;
    }

    bool call(Value::Function* func, int argCount) {
        if (argCount != func->arity) {
            std::cerr << std::format("Internal error: Function \"{:s}\" expected {} arguments, got {} instead", func->name, func->arity, argCount) << std::endl;
            return false;
        }

        if (fp == FRAME_COUNT) {
            std::cerr << "Stack overflow" << std::endl;
            std::cerr << printCallStack();
            return false;
        }

        CallFrame* frame = &callStack[fp++];
        frame->slots = &stack[sp - argCount];
        frame->returnip = ip;
        frame->func = func;

        ip = func->code.data();

        return true;
    }

    bool callNative(Value::NativeFn* func, int argCount) {
        if (argCount != func->arity) {
            std::cerr << std::format("Internal error: Function \"{:s}\" expected {} arguments, got {} instead", func->name, func->arity, argCount) << std::endl;
            return false;
        }

        Value::Value returnValue = func->fn(&stack[sp - argCount]);

        sp -= argCount + 1; // +1 to pop the function object
        push(returnValue);

        return true;

    }

    bool callValue(Value::Value callee, int argCount) {

        switch (callee.type) {
            case Value::OBJECT: {
                Value::Obj* obj = callee.as.object;

                switch (obj->type) {
                    case Value::FUNCTION:
                        return call(static_cast<Value::Function*>(obj), argCount);
                    case Value::NATIVE_FN:
                        return callNative(static_cast<Value::NativeFn*>(obj), argCount);

                    default:
                        break;
                }
            }

            default:
                break;
        }

        std::cerr << "Internal error: Can only call functions." << std::endl;
        return false;
    }

    std::string printCallStack() {
        std::string output;

        for (i64 i = fp - 1; i >= 0; i--) {
            CallFrame frame = callStack[i];
            u8* location = i == fp - 1 ? ip : callStack[i + 1].returnip;
            output += frame.func->name + std::format(" at 0x{:04X}\n", location - frame.func->code.data());
        }

        return output;
    }
    
public:
    explicit VM(Program& program): ip(nullptr), sp(0), fp(0), program(program) {
        ip = this->program.code.data();
        globals = (Value::Value*) malloc(sizeof(Value::Value) * this->program.globalCount);
#ifndef BYTECODE_SAFETY
        constants = this->program.constants.data();
#endif
    }

    i32 interpret() {
        while (true) {
            Operation op = READ_OP();

            switch (op) {
                case RETURN: {
                    Value::Value returnValue = pop();

                    // restore old call frame and ip
                    fp--;
                    ip = callStack[fp].returnip;

                    // pop local vars + params
                    Value::Value* stackTop = &stack[sp + 1]; // leave pointing just past actual top to account for function object still sitting underneath params
                    u16 diff = stackTop - callStack[fp].slots;

                    sp -= diff;


                    push(returnValue);

                    break;
                }

                case PRINT: {
                    Value::printValue(pop());
                    break;
                }

#define OPERATION(OPERATOR) do {\
                    f64 b = AS_NUM(pop());\
                    f64 a = AS_NUM(pop());\
                    Value::Value result = VALUE_NUM(a OPERATOR b); \
                    push(result);\
                } while(false)

                case ADD: OPERATION(+); break;
                case SUBTRACT: OPERATION(-); break;
                case MULTIPLY: OPERATION(*); break;
                case DIVIDE: OPERATION(/); break;
#undef OPERATION

                case NEGATE: push(VALUE_NUM(-AS_NUM(pop()))); break;
                case NOT: push(VALUE_BOOL(!Value::isTruthy(pop()))); break;

                case EQUALS: {
                    Value::Value b = pop();
                    Value::Value a = pop();
                    push(VALUE_BOOL(Value::equality(a, b)));
                    break;
                }

#define COMPARE(OPERATOR) do {\
                    f64 b = AS_NUM(pop());\
                    f64 a = AS_NUM(pop());\
                    Value::Value result = VALUE_BOOL(a OPERATOR b); \
                    push(result);\
                } while(false)


                case LESS: COMPARE(<); break;
                case MORE: COMPARE(>); break;
                case LESS_EQUALS: COMPARE(<=); break;
                case MORE_EQUALS: COMPARE(>=); break;
#undef COMPARE

                case CALL: {
                    u8 argCount = READ_BYTE();
                    Value::Value func = peek(argCount);

                    if (!callValue(func, argCount)) return EX_DATAERR;
                    break;
                }

                case POP: {
                    pop();
                    break;
                }

                case POP_N: {
                    u8 count = READ_BYTE();
                    sp -= count;
                    break;
                }

                case LOAD_I8: {

                    i8 value = std::bit_cast<i8>(READ_BYTE()); // TODO: check if this can be replaced with a manual bit cast and if that would be faster

                    push(VALUE_NUM(value));

                    break;
                }

                case TRUE: {
                    push(VALUE_TRUE);
                    break;
                }

                case FALSE: {
                    push(VALUE_FALSE);
                    break;
                }

                case LOAD_CONSTANT: {
                    #ifdef BYTECODE_SAFETY
                        push(program.constants.at(READ_BYTE()));
                    #else
                        push(constants[READ_BYTE()]);
                    #endif
                    break;
                }

                case LOAD_GLOBAL: {
                    const u8 index = READ_BYTE();
                    push(globals[index]);
                    break;
                }

                case LOAD_LOCAL: {
                    u8 slot = READ_BYTE();
                    push(callStack[fp - 1].slots[slot]);
                    break;
                }

                case SET_GLOBAL: {
                    u8 slot = READ_BYTE();
                    globals[slot] = peek(0);
                    break;
                }

                case SET_LOCAL: {
                    u8 slot = READ_BYTE();
                    callStack[fp - 1].slots[slot] = peek(0);
                    break;
                }

                case JUMP: {
                    u16 distance = READ_SHORT();
                    ip += distance;
                    break;
                }

                case JUMP_FALSE: {
                    u16 distance = READ_SHORT();
                    if (!Value::isTruthy(peek(0)))
                        ip += distance;
                    break;
                }

                case JUMP_TRUE: {
                    u16 distance = READ_SHORT();
                    if (Value::isTruthy(peek(0)))
                        ip += distance;
                    break;
                }

                case LOOP: {
                    u16 distance = READ_SHORT();
                    ip -= distance;
                    break;
                }

                case EXIT: {
                    Value::Value returnValue = pop();
#ifdef BYTECODE_SAFETY
                    if (sp != 0) std::cerr << "stack was not empty, included " << sp << " values" << std::endl;
#endif
                    return AS_NUM(returnValue);
                }

            }
        }


    }
};

i32 Bytecode::interpret(const Compiler::CompileResult &ast, bool decompile) {
    Program program;
    try {
        program = BytecodeCompilation::compile(ast);
    } catch (BytecodeCompilation::CompileError& e) {
        std::cerr << e.message << std::endl;
        return EX_DATAERR;
    }

    if (decompile) Decompile::decompile(program);

    VM vm(program);
    return vm.interpret();
}
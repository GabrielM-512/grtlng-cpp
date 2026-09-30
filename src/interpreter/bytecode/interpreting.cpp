#include "interpreting.h"

#include <iostream>
#include <utility>

#include "compiler.h"
#include "decompile.h"

using namespace Bytecode;

#define READ_BYTE() (*ip++)
#define READ_SHORT() (ip += 2, (u16)((ip[-2] << 8) | ip[-1]))
#define READ_OP() (static_cast<Operation>(READ_BYTE()))

#define BYTECODE_SAFETY

class VM {
    u8* ip;
    int sp;

#ifndef BYTECODE_SAFETY
    Value::Value* constants;
#endif

    Program program;
    Value::Value stack[256] {};
    Value::Value *globals;

    Value::Value pop() {
        return stack[--sp];
    }
    
    [[nodiscard]] Value::Value peek(u16 dist) const {
        return stack[sp - dist - 1];
    }
    
    void push(Value::Value value) {
        stack[sp++] = value;
    }
    
public:
    explicit VM(Program program): ip(nullptr), sp(0), program(std::move(program)) {
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
                    return AS_NUM(pop());
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
                    push(stack[slot]);
                    break;
                }

                case SET_GLOBAL: {
                    u8 slot = READ_BYTE();
                    globals[slot] = peek(0);
                    break;
                }

                case SET_LOCAL: {
                    u8 slot = READ_BYTE();
                    stack[slot] = peek(0);
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
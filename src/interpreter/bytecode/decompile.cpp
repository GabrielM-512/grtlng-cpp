#include "decompile.h"
#ifndef INTERPRETER_AST
#include <iostream>

using namespace Decompile;

void printOffsetInstruction(u64 offset, const std::string& instruction, bool pipe) {
    std::string message = std::format("  0x{:04X} | ", offset) + instruction;
    if (pipe) message +=  " | ";

    std::cout << message;
}

[[nodiscard]] u64 i8Instruction(u64 offset, const std::string& instruction, const std::vector<u8>& code) {

    i8 value = std::bit_cast<i8>(code.at(offset + 1));

    printOffsetInstruction(offset, instruction, true);
    std::cout << (int) value << std::endl;

    return offset + 2;
}

[[nodiscard]] u64 u8Instruction(u64 offset, const std::string& instruction, const std::vector<u8>& code) {
    u8 value = code.at(offset + 1);

    printOffsetInstruction(offset, instruction, true);
    std::cout << (int) value << std::endl;

    return offset + 2;
}

[[nodiscard]] u64 u16Instruction(u64 offset, const std::string& instruction, const std::vector<u8>& code) {
    u16 value = (u16)(code[offset + 1] << 8);
    value |= code[offset + 2];

    printOffsetInstruction(offset, instruction, true);
    std::cout << value << std::endl;

    return offset + 3;
}

[[nodiscard]] u64 constantInstruction(u64 offset, const std::string& instruction, Bytecode::Program program, std::vector<u8> code) {
    u8 index = code.at(offset + 1);
    Value::Value value = program.constants.at(index);

    printOffsetInstruction(offset, instruction, true);
    std::cout << (int) index << " | " << Value::getValueString(value) << std::endl;

    return offset + 2;
}

[[nodiscard]] u64 simpleInstruction(u64 offset, const std::string& instruction) {
    printOffsetInstruction(offset, instruction, false);
    std::cout << std::endl;
    return offset + 1;
}

void decompileChunk(std::vector<u8> code, const std::string& name, const Bytecode::Program& program) {
    u64 offset = 0;

    std::cout << name << ":" << std::endl;

    while (offset < code.size()) {
        auto op = static_cast<Bytecode::Operation>(code.at(offset));
        switch (op) {
            case Bytecode::RETURN: {
                offset = simpleInstruction(offset, "RETURN");
                break;
            }

            case Bytecode::PRINT: {
                offset = simpleInstruction(offset, "PRINT");
                break;
            }

            case Bytecode::ADD: {
                offset = simpleInstruction(offset, "ADD");
                break;
            }

            case Bytecode::SUBTRACT: {
                offset = simpleInstruction(offset, "SUBTRACT");
                break;
            }

            case Bytecode::MULTIPLY: {
                offset = simpleInstruction(offset, "MULTIPLY");
                break;
            }

            case Bytecode::DIVIDE: {
                offset = simpleInstruction(offset, "DIVIDE");
                break;
            }

            case Bytecode::NEGATE: {
                offset = simpleInstruction(offset, "NEGATE");
                break;
            }

            case Bytecode::NOT: {
                offset = simpleInstruction(offset, "NOT");
                break;
            }

            case Bytecode::EQUALS: {
                offset = simpleInstruction(offset, "EQUALS");
                break;
            }

            case Bytecode::LESS: {
                offset = simpleInstruction(offset, "LESS");
                break;
            }

            case Bytecode::MORE: {
                offset = simpleInstruction(offset, "MORE");
                break;
            }

            case Bytecode::LESS_EQUALS: {
                offset = simpleInstruction(offset, "LESS_EQUALS");
                break;
            }

            case Bytecode::MORE_EQUALS: {
                offset = simpleInstruction(offset, "MORE_EQUALS");
                break;
            }

            case Bytecode::CALL: {
                offset = u8Instruction(offset, "CALL", code);
                break;
            }

            case Bytecode::POP: {
                offset = simpleInstruction(offset, "POP");
                break;
            }

            case Bytecode::POP_N: {
                offset = u8Instruction(offset, "POP_N", code);
                break;
            }

            case Bytecode::LOAD_I8: {
                offset = i8Instruction(offset, "LOAD_I8", code);
                break;
            }

            case Bytecode::FALSE: {
                offset = simpleInstruction(offset, "FALSE");
                break;
            }

            case Bytecode::TRUE: {
                offset = simpleInstruction(offset, "TRUE");
                break;
            }

            case Bytecode::LOAD_CONSTANT: {
                offset = constantInstruction(offset, "LOAD_CONSTANT", program, code);
                break;
            }

            case Bytecode::LOAD_GLOBAL: {
                offset = u8Instruction(offset, "LOAD_GLOBAL", code);
                break;
            }

            case Bytecode::LOAD_LOCAL: {
                offset = u8Instruction(offset, "LOAD_LOCAL", code);
                break;
            }

            case Bytecode::SET_GLOBAL: {
                offset = u8Instruction(offset, "SET_GLOBAL", code);
                break;
            }

            case Bytecode::SET_LOCAL: {
                offset = u8Instruction(offset, "SET_LOCAL", code);
                break;
            }

            case Bytecode::JUMP: {
                offset = u16Instruction(offset, "JUMP", code);
                break;
            }

            case Bytecode::JUMP_FALSE: {
                offset = u16Instruction(offset, "JUMP_FALSE", code);
                break;
            }

            case Bytecode::JUMP_TRUE: {
                offset = u16Instruction(offset, "JUMP_TRUE", code);
                break;
            }

            case Bytecode::LOOP: {
                offset = u16Instruction(offset, "LOOP", code);
                break;
            }

            case Bytecode::EXIT: {
                offset = simpleInstruction(offset, "EXIT");
                break;
            }


            default:
                std::cout << "Unknown Operation " << (i32) op << std::endl;
                offset++;
        }
    }

    std::cout << std::endl;
}

void Decompile::decompile(const Bytecode::Program& program) {
    decompileChunk(program.code, "init", program);

    for (Value::Value obj : program.constants) {
        if (IS_FUNC(obj)) {
            Value::Function* func = AS_FUNCTION(obj);
            decompileChunk(func->code, func->name, program);
        }
    }

    std::cout << std::endl << std::endl;
}

#endif
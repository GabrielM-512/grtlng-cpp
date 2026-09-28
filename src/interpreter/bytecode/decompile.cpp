#include "decompile.h"

#include <iostream>

using namespace Decompile;

[[nodiscard]] u64 i8Instruction(u64 offset, const std::string& instruction, std::vector<u8>& code) {

    i8 value = std::bit_cast<i8>(code.at(offset + 1));

    std::cout << instruction << " | " << (int) value << std::endl;

    return offset + 2;
}

[[nodiscard]] u64 u8Instruction(u64 offset, const std::string& instruction, std::vector<u8>& code) {
    u8 value = code.at(offset + 1);
    std::cout << instruction << " | " << (int) value << std::endl;

    return offset + 2;
}

[[nodiscard]] u64 constantInstruction(u64 offset, const std::string& instruction, Bytecode::Program program) {
    u8 index = program.code.at(offset + 1);
    Value::Value value = program.constants.at(index);

    std::cout << instruction << " | " << (int) index << " | " << Value::getValueString(value) << std::endl;

    return offset + 2;
}

[[nodiscard]] u64 simpleInstruction(u64 offset, const std::string& instruction) {
    std::cout << instruction << std::endl;
    return offset + 1;
}

void Decompile::decompile(Bytecode::Program program) {
    u64 offset = 0;

    while (offset < program.code.size()) {
        auto op = static_cast<Bytecode::Operation>(program.code.at(offset));
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

            case Bytecode::POP: {
                offset = simpleInstruction(offset, "POP");
                break;
            }

            case Bytecode::POP_N: {
                offset = u8Instruction(offset, "POP_N", program.code);
                break;
            }

            case Bytecode::LOAD_I8: {
                offset = i8Instruction(offset, "LOAD_I8", program.code);
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
                offset = constantInstruction(offset, "LOAD_CONSTANT", program);
                break;
            }

            case Bytecode::LOAD_GLOBAL: {
                offset = u8Instruction(offset, "LOAD_GLOBAL", program.code);
                break;
            }

            case Bytecode::LOAD_LOCAL: {
                offset = u8Instruction(offset, "LOAD_LOCAL", program.code);
                break;
            }

            case Bytecode::SET_GLOBAL: {
                offset = u8Instruction(offset, "SET_GLOBAL", program.code);
                break;
            }

            case Bytecode::SET_LOCAL: {
                offset = u8Instruction(offset, "SET_LOCAL", program.code);
                break;
            }


            default:
                std::cout << "Unknown Operation " << (i32) op << std::endl;
                offset++;
        }
    }

    std::cout << std::endl << std::endl;
}
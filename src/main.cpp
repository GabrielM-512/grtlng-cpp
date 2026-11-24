#include <iostream>

#include "compiler/compiler.h"

#include "interpreter/interpreter.h"

#include "argvParse.h"
#include "util/programDump.h"

#define FREE_PATHS() do {\
    if (compileFlags.dumpPath != nullptr) free(compileFlags.dumpPath);\
    if (compileFlags.filePath != nullptr) free(compileFlags.filePath);\
} while (false)

int main(const int argc, char* argv[]) {
    argvParse::ProgramArgs compileFlags = argvParse::parse(argc, argv);
    Bytecode::Program bcProgram;

    if (compileFlags.dump == argvParse::LOAD) {
        try {
            bcProgram = programDump::readBytecodeFile(compileFlags.filePath);
        } catch (programDump::LoadError& e) {
            std::cerr << "Error while loading Bytecode Program: " << e.what() << std::endl;
            return 1;
        }
    } else {
        bcProgram = Compiler::compile(compileFlags);
    }

    if (compileFlags.dump == argvParse::DUMP) {
        programDump::dumpBytecode(bcProgram, compileFlags.dumpPath);
    }

    FREE_PATHS();

    switch (compileFlags.type) {
        case argvParse::INTERPRET: {
            double result = Bytecode::interpret(bcProgram, compileFlags.decompile);
            return (int) result;
        }

        case argvParse::COMP_NONE: return 0;

    }
}

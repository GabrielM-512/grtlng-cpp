#include <iostream>

#include "../tool/prettyPrinter.h"
#include "compiler/compiler.h"
#include "util/fileIO.h"

#include "interpreter/bytecode/interpreting.h"

#include "argvParse.h"
#include "interpreter/bytecode/compiler.h"
#include "util/programDump.h"

Bytecode::Program compileProgram(const argvParse::ProgramArgs& compileFlags) {
    std::string file = fileIO::readFile(compileFlags.filePath);

    Error::ErrorHandler handler(file);

    Compiler::CompileResult program = Compiler::compile(file, handler);

    if (!program.success) {
        handler.printErrors();
        exit(1);
    }

    if (compileFlags.printAst) {
        std::cout << Printer::print(program) << std::endl;
    }

    Bytecode::Program bcProgram;

    try {
        bcProgram = BytecodeCompilation::compile(program);
    } catch (BytecodeCompilation::CompileError& e) {
        std::cerr << e.message << std::endl;
        exit(EX_DATAERR);
    }

    return bcProgram;
}

int main(const int argc, char* argv[]) {
    argvParse::ProgramArgs compileFlags = argvParse::parse(argc, argv);
    Bytecode::Program bcProgram;

    if (compileFlags.dump == argvParse::LOAD) {
        try {
            bcProgram = programDump::readBytecode(compileFlags.filePath);
        } catch (programDump::LoadError& e) {
            std::cerr << "Error while loading Bytecode Program: " << e.what() << std::endl;
            return 1;
        }
    } else {
        bcProgram = compileProgram(compileFlags);
    }

    if (compileFlags.dump == argvParse::DUMP) {
        programDump::dumpBytecode(bcProgram, compileFlags.dumpPath);
    }

    switch (compileFlags.type) {
        case argvParse::INTERPRET: {
            double result = Bytecode::interpret(bcProgram, compileFlags.decompile);
            return (int) result;
        }

        case argvParse::COMP_NONE: return 0;

    }
}

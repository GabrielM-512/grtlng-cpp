#include <iostream>

#include "compiler/compiler.h"
#include "interpreter/bytecode/interpreting.h"
#include "util/fileIO.h"

#include "argvParse.h"

//#define CATCH_ERRORS

int main(const int argc, char* argv[]) {
    argvParse::ProgramArgs compileFlags = argvParse::parse(argc, argv);
#ifdef CATCH_ERRORS
    try {
#endif
        std::string file = fileIO::readFile(compileFlags.filePath);

        Error::ErrorHandler handler(file);

        Compiler::CompileResult program = Compiler::compile(file, handler);

        if (!program.success) {
            handler.printErrors();
            return 1;
        }

        if (compileFlags.interpret) {
            double result = Bytecode::interpret(program, compileFlags.decompile);

            return (int) result;
        }

        return 0;
#ifdef CATCH_ERRORS
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
#endif
    return 0;
}

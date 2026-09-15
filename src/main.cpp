#include <iostream>

#include "compiler/compiler.h"
#include "interpreter/interpreting.h"
#include "util/fileIO.h"

#include "argvParse.h"


int main(const int argc, char* argv[]) {
    argvParse::ProgramArgs compileFlags = argvParse::parse(argc, argv);

    try {
        std::string file = fileIO::readFile(compileFlags.filePath);

        Error::ErrorHandler handler(file);

        Compiler::CompileResult program = Compiler::compile(file, handler);

        if (!program.success) return 1;

        if (compileFlags.interpret) {
            double result = Interpreting::interpret(program, handler);

            return (int) result;
        }

        return 0;

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

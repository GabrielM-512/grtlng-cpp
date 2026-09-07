#include <iostream>
#include <sysexits.h>

#include "../tool/prettyPrinter.h"
#include "compiler/compiler.h"
#include "interpreter/interpreting.h"
#include "util/fileIO.h"

struct parseFlags {
    bool interpret;
    char *filePath;

    bool flagsOK;
};

parseFlags parseArgs(const int argc, char* argv[]) {
    parseFlags flags = {.interpret = true, .filePath = nullptr, .flagsOK = true};

    if (argc != 2) {
        std::cerr << "Improper Usage" << std::endl << "  Proper usage: ./grtlng <input_file>";
        flags.flagsOK = false;
    }

    flags.filePath = argv[1];

    return flags;
}

int main(const int argc, char* argv[]) {
    parseFlags compileFlags;
    if (!(compileFlags = parseArgs(argc, argv)).flagsOK) exit(EX_USAGE);

    try {
        const std::string file = fileIO::readFile(compileFlags.filePath);
        Compiler::CompileResult program = Compiler::compile(file);

        if (!program.success) return 1;

        std::cout << Printer::print(program.expr) << std::endl;
        double result = Interpreting::interpret(program.expr);
        std::cout << "Result: " << result << std::endl;
        return (int) result;

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#include "compiler.h"

#include <iostream>

#include "lexing.h"
#include "resolving.h"
#include "parsing/parsing.h"

#include "../util/fileIO.h"
#include "bytecode.h"

#include "../../tool/prettyPrinter.h"

Bytecode::Program Compiler::compile(const argvParse::ProgramArgs& flags) {
    std::string file = fileIO::readFile(flags.filePath);

    Error::ErrorHandler errorHandler(file);

    SyntaxTree tree;

    std::vector<Lexing::Token> tokens = Lexing::scan(file);

    Parsing::Parser parser(tokens, errorHandler);
    tree.tree = parser.parse();

    if (parser.hadParseError()) tree.success = false;
    if (parser.hadFatalParseError()) {
        errorHandler.printErrors();
        exit(1);
    }

    Resolving::resolve(tree, errorHandler);

    if (!tree.success) {
        errorHandler.printErrors();
        exit(1);
    }

    if (flags.printAst) {
        std::cout << Printer::print(tree) << std::endl;
    }

    Bytecode::Program bcProgram;
    try {
        Bytecode::BytecodeCompiler compiler;
        bcProgram = compiler.compile(tree);
    } catch (CompileError& e) {
        std::cerr << e.message << std::endl;
        exit(EX_DATAERR);
    }
    // free the AST
    for (Stmt::Stmt * current : tree.tree) delete current;

    return bcProgram;
}

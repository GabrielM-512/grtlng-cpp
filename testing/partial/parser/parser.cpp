#include <vector>
#include <iostream>

#include "../../../src/util/fileIO.h"
#include "../../../src/compiler/lexing.h"
#include "../../../src/compiler/parsing/parsing.h"
#include "../../../tool/prettyPrinter.h"
#include "../../../src/error.h"


static const char *filepath = "/home/gabriel/CLionProjects/grtlng-cpp/testing/partial/parser/test.gl";

int main() {
    std::string file = fileIO::readFile(filepath);
    std::vector<Lexing::Token> tokens = Lexing::scan(file);

    Error::ErrorHandler handler(file);

    Parsing::Parser parser(tokens, handler);
    std::vector<Stmt::Stmt*> program = parser.parse();

    std::string target = "[EXPR] ( ( 16 + ( 5 * 3 ) ) - a )";
    std::string got = Printer::print(program);

    if (!target.compare(got)) {
        std::cerr << "Parsing: Failed" << std::endl <<
                     "    Expected: " << target << std::endl <<
                     "    Got:      " << got << std::endl;
        return 1;
    }
    std::cout << "Parsing: successful" << std::endl;
    return 0;
}
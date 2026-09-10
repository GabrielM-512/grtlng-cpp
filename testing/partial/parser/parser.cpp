#include <vector>
#include <iostream>

#include "../../../src/AST/expr.h"
#include "../../../src/util/fileIO.h"
#include "../../../src/compiler/lexing.h"
#include "../../../src/compiler/parsing/parsing.h"
#include "../../../tool/prettyPrinter.h"


static const char *filepath = "/home/gabriel/CLionProjects/grtlng-cpp/testing/partial/parser/test.gl";

int main() {
    std::string file = fileIO::readFile(filepath);
    std::vector<Lexing::Token> tokens = Lexing::scan(file);

    Parsing::Parser parser(tokens);
    Expr::Expr* expr = parser.parse();

    std::string target = "( ( 16 + ( 5 * 3 ) ) - a )";
    std::string got = Printer::print(expr);

    if (target != got) {
        std::cerr << "Parsing: Failed" << std::endl <<
                     "    Expected: " << target << std::endl <<
                     "    Got:      " << got << std::endl;
        return 1;
    }
    std::cout << "Parsing: successful" << std::endl;
    return 0;
}
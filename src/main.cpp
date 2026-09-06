#include <iostream>
#include <sysexits.h>

#include "compiler/compiler.h"
#include "compiler/lexing.h"
#include "util/fileIO.h"

bool checkArgs(const int argc, char* []) {
    if (argc != 2) {
        std::cerr << "Improper Usage" << std::endl << "  Proper usage: ./grtlng <input_file>";
        return false;
    }
    return true;
}

int main(const int argc, char* argv[]) {
    if (!checkArgs(argc, argv)) exit(EX_USAGE);

    try {
        const std::string file = fileIO::readFile(argv[1]);
        const std::vector<Lexing::Tokens::Token> tokens = Lexing::scan(file);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

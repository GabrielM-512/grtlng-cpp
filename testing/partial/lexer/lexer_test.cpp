#include <cstring>
#include <cstdio>
#include <vector>
#include <iostream>

#include "../../../src/compiler/lexing.h"
#include "../../../src/util/fileIO.h"

static const char *filepath = "/home/gabriel/CLionProjects/grtlng-cpp/testing/partial/lexer/test.gl";

void printTokenFile(const Lexing::Tokens::Token& token, FILE* output) {
    using namespace Lexing::Tokens;
    fprintf(output, "{%d, %d, %d, {", token.type, token.line, token.position);
    switch (token.type) {
        case NUMBER:
            fprintf(output, ".number = %f", token.data.number);
            break;
        case IDENTIFIER:
        case ERROR:
        case STRING:
            fprintf(output, ".name = (char*) \"%s\"", (char*) token.data.name);
            break;
        default:
            fprintf(output, ".name = nullptr");
    }

    fprintf(output, "}},\n");
}

void printToken(const Lexing::Tokens::Token& token) {
    printTokenFile(token, stdout);
}

void printTokenError(const Lexing::Tokens::Token& token) {
    printTokenFile(token, stderr);
}

int main() {
    std::string file = fileIO::readFile(filepath);
    std::vector<Lexing::Tokens::Token> tokens = Lexing::scan(file);

    using namespace Lexing::Tokens;
    Token compareTarget[] = {
        {IDENTIFIER, 1, 0, {.name = (char*) "andy"}},
        {IDENTIFIER, 1, 5, {.name = (char*) "formless"}},
        {IDENTIFIER, 1, 14, {.name = (char*) "fo"}},
        {IDENTIFIER, 1, 17, {.name = (char*) "_"}},
        {IDENTIFIER, 1, 19, {.name = (char*) "_123"}},
        {IDENTIFIER, 1, 24, {.name = (char*) "_abc"}},
        {IDENTIFIER, 1, 29, {.name = (char*) "ab123"}},
        {IDENTIFIER, 2, 35, {.name = (char*) "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_"}},
        {ELSE, 4, 100, {.name = nullptr}},
        {FALSE, 4, 105, {.name = nullptr}},
        {FOR, 4, 111, {.name = nullptr}},
        {IF, 4, 115, {.name = nullptr}},
        {RETURN, 4, 118, {.name = nullptr}},
        {TRUE, 4, 125, {.name = nullptr}},
        {WHILE, 4, 130, {.name = nullptr}},
        {EXTERN, 4, 136, {.name = nullptr}},
        {PRINT, 4, 143, {.name = nullptr}},
        {U8, 6, 150, {.name = nullptr}},
        {U16, 6, 153, {.name = nullptr}},
        {U32, 6, 157, {.name = nullptr}},
        {U64, 6, 161, {.name = nullptr}},
        {I8, 6, 165, {.name = nullptr}},
        {I16, 6, 168, {.name = nullptr}},
        {I32, 6, 172, {.name = nullptr}},
        {I64, 6, 176, {.name = nullptr}},
        {F32, 6, 180, {.name = nullptr}},
        {F64, 6, 184, {.name = nullptr}},
        {VOID, 6, 188, {.name = nullptr}},
        {ERROR, 8, 194, {.name = (char*) "Unexpected character."}},
        {STRING, 10, 197, {.name = (char*) "hahahoho"}},
        {NUMBER, 11, 208, {.number = 1.000000}},
        {NUMBER, 12, 210, {.number = 2.000000}},
        {NUMBER, 13, 212, {.number = 3.000000}},
        {SEMICOLON, 15, 215, {.name = nullptr}},
        {LEFT_PAREN, 17, 218, {.name = nullptr}},
        {RIGHT_PAREN, 17, 219, {.name = nullptr}},
        {LEFT_BRACE, 18, 221, {.name = nullptr}},
        {RIGHT_BRACE, 18, 222, {.name = nullptr}},
        {LEFT_BRACKET, 19, 224, {.name = nullptr}},
        {RIGHT_BRACKET, 19, 225, {.name = nullptr}},
        {PLUS, 21, 228, {.name = nullptr}},
        {MINUS, 21, 230, {.name = nullptr}},
        {STAR, 21, 232, {.name = nullptr}},
        {SLASH, 21, 234, {.name = nullptr}},
        {PLUS_EQUALS, 22, 236, {.name = nullptr}},
        {MINUS_EQUALS, 22, 239, {.name = nullptr}},
        {STAR_EQUALS, 22, 242, {.name = nullptr}},
        {SLASH_EQUALS, 22, 245, {.name = nullptr}},
        {PLUS_PLUS, 23, 248, {.name = nullptr}},
        {MINUS_MINUS, 23, 251, {.name = nullptr}},
        {AMP, 24, 254, {.name = nullptr}},
        {PIPE, 24, 256, {.name = nullptr}},
        {TILDE, 24, 258, {.name = nullptr}},
        {AMP_AMP, 25, 260, {.name = nullptr}},
        {PIPE_PIPE, 25, 263, {.name = nullptr}},
        {AMP_EQUALS, 26, 266, {.name = nullptr}},
        {PIPE_EQUALS, 26, 269, {.name = nullptr}},
        {BANG, 27, 272, {.name = nullptr}},
        {DOT, 27, 274, {.name = nullptr}},
        {COMMA, 27, 276, {.name = nullptr}},
        {MORE, 28, 278, {.name = nullptr}},
        {LESS, 28, 280, {.name = nullptr}},
        {EQUALS, 28, 282, {.name = nullptr}},
        {EQUALS_EQUALS, 28, 284, {.name = nullptr}},
        {MORE_EQUALS, 28, 287, {.name = nullptr}},
        {LESS_EQUALS, 28, 290, {.name = nullptr}},
        {BANG_EQUALS, 28, 293, {.name = nullptr}},
        {END_OF_FILE, 29, 296, {.name = nullptr}},
    };

    size_t compLength = std::size(compareTarget);
    if (compLength != tokens.size()) {
        std::cout << "Lengths didn't match: expected " << compLength << ", got " << tokens.size() << " instead" << std::endl;

        std::cout << "Got: " << std::endl;
        for (size_t i = 0; i < tokens.size(); i++) {
            Token t = tokens.at(i);
            std::cout << i + 1 << ": ";
            printToken(t);
        }

        std::cout << std::endl << std::endl << std::endl << "Expected: " << std::endl;

        for (size_t i = 0; i < compLength; i++) {
            Token t = compareTarget[i];
            std::cout << i + 1 << ": ";
            printToken(t);
        }

        return 1;
    }

    for (size_t i = 0; i < compLength; i++) {
        Token got = tokens[i];
        Token should = compareTarget[i];

        bool matches = true;
        if (got.type != should.type) matches = false;
        if (got.line != should.line) matches = false;
        if (got.position != should.position) matches = false;

        switch (got.type) {
            case IDENTIFIER:
            case STRING:
            case ERROR:
                if (strcmp(got.data.name, should.data.name) != 0) matches = false;
                break;
            default:
                if (got.data.number != should.data.number) matches = false;
        }

        if (!matches) {
            std::cerr << "Token # " << i << " didn't match:" << std::endl;
            std::cerr << "\tExpected: ";
            printTokenError(should);
            std::cerr << "\tGot:      ";
            printTokenError(got);
            return 1;
        }
    }

    std::cout << "Lexing: Success" << std::endl;

    return 0;
}
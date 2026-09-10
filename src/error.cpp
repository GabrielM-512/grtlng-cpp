#include "error.h"

#include <string>
#include <format>

#include <iostream>
#include <iomanip>
#include <utility>


using namespace Error;

#define MAX_ERROR_LINE_LENGTH 20

void ErrorHandler::printErrorLine(const Lexing::Token token) const {
    u32 start = token.position;

    while (start > 0 && source[start - 1] != '\n') start--;

    u32 end = token.position;
    for (u8 i = 0; i < MAX_ERROR_LINE_LENGTH; i++) {
        end++;
        if (end >= source.length() || source[end] == '\n') break;
    }

    u32 range = end - start;

    std::string line = source.substr(start, range);
    if (token.position + MAX_ERROR_LINE_LENGTH == end) line += "...";

    std::cerr << "[" << token.line << "]    " << line << std::endl;

    const u16 arrowStart = token.position - start + 6 + std::to_string(token.line).length();

    std::cerr << std::format("{:{}s}^ Here", "", arrowStart) << std::endl;
}

void ErrorHandler::runtimeError(Interpreting::RuntimeException &error) {
    std::cerr << "Runtime Exception on line " << error.token.line << ": " << error.message << std::endl;
    printErrorLine(error.token);
}

void ErrorHandler::compileError(std::string message, std::string hint, Lexing::Token token) {
    CompileError error(std::move(message), std::move(hint), token);
    compileErrors.push_back(error);

}
void ErrorHandler::printErrors() {
    for (u64 i = 0; i < compileErrors.size(); i++) {
        CompileError& error = compileErrors[i];
        std::cerr << "Encountered error on line " << error.token.line << ": " << error.message << std::endl;
        printErrorLine(error.token);
        if (!error.hint.empty()) {
            std::cerr << std::endl << "Hint: " << error.hint << std::endl;
        }

        bool isLast = i + 1 >= compileErrors.size();
        if (!isLast) std::cerr << std::endl << std::endl << std::endl;
    }
}
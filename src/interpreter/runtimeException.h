#pragma once
#include <stdexcept>

#include "../compiler/lexing.h"

namespace Interpreting {
    class RuntimeException : public std::runtime_error {
    public:
        std::string message;
        Lexing::Token token;
        RuntimeException(const std::string& message, Lexing::Token token) : std::runtime_error(message), message(message), token(token) {}
        RuntimeException(const std::string& message) : std::runtime_error(message), message(message), token((Lexing::Token) {.type = Lexing::ERROR, .line = 0, .position = 0, .data = {nullptr}}) {}
    };
}

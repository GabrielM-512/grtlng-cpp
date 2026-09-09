#pragma once
#include <stdexcept>

namespace Interpreting {
    class RuntimeException : public std::runtime_error {
    public:
        std::string message;
        Lexing::Tokens::Token token;
        RuntimeException(const std::string message, Lexing::Tokens::Token token) : std::runtime_error(message), message(message), token(token) {}
    };
}

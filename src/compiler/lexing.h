#pragma once

#include <vector>

#include "../global.h"

namespace Lexing {

    typedef enum : u16{
        END_OF_FILE, // End of source file
        ERROR,

        NUMBER, // Any number literal
        STRING, // Any String
        FALSE,
        TRUE,

        SEMICOLON,

        LEFT_PAREN, // (
        RIGHT_PAREN, // )
        LEFT_BRACE, // {
        RIGHT_BRACE, // }
        LEFT_BRACKET, // [
        RIGHT_BRACKET, // ]

        PLUS,
        MINUS,
        STAR,
        SLASH,

        PLUS_EQUALS,
        MINUS_EQUALS,
        STAR_EQUALS,
        SLASH_EQUALS,

        PLUS_PLUS,
        MINUS_MINUS,

        AMP, // '&'
        PIPE, // '|'
        TILDE, // '~'

        AMP_AMP, // &&
        PIPE_PIPE, // ||

        AMP_EQUALS, // &=
        PIPE_EQUALS, // |=

        BANG, // '!'
        DOT,
        COMMA,

        MORE,
        LESS,

        EQUALS,
        EQUALS_EQUALS,
        MORE_EQUALS,
        LESS_EQUALS,
        BANG_EQUALS,

        IDENTIFIER,

        // type tokens
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        F32,
        F64,
        VOID,

        // keyword tokens
        RETURN,
        EXTERN,

        IF,
        ELSE,

        WHILE,
        FOR,

        PRINT,

        LAST // a marker token to always be the last token
    } TokenType;

    struct Token {
        TokenType type;
        u16 line;
        u32 position;
        union {
            const char* name;
            double number;
        } data;

        [[nodiscard]] std::string toString() const;
        static std::string toString(TokenType tokenType);

        };

    class ScanException : public std::runtime_error {
    public:
        ScanException(const std::string& message) : std::runtime_error(message) {}
    };


    std::vector<Token> scan(const std::string& input);
}
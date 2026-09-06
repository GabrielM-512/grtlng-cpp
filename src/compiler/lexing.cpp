#include "lexing.h"

#include <cstring>

using namespace Lexing::Tokens;

class Lexer {
    std::string source;
    u32 base, head;
    u16 line;

    [[nodiscard]] char peek() const;
                  char advance();
    [[nodiscard]] bool isAtEnd() const;
    [[nodiscard]] char peekNext() const;

                  Token makeToken(TokenType type, const char* data) const;
                  Token makeToken(TokenType type, const char* data, u16 beginningLine) const;
    [[nodiscard]] Token makeToken(TokenType type, double number) const;
    [[nodiscard]] Token noDataToken(TokenType type) const;
                  Token errorToken(const char* message) const;
                  Token errorToken(const char *message, u16 beginningLine) const;

                  Token number();
                  Token string();
                  Token checkKeyword(u16 start, const char *remaining, TokenType type) const;
    [[nodiscard]] Token keyword() const;
                  Token identifier();
                  bool comment();

public:
    explicit Lexer(const std::string& input) {
        source = input;
        base = head = 0;
        line = 1;
    }

    void skipWhitespace();
    Token scanToken();

};

// utils

static bool isAlpha(const char c) {
    return ('a' <= c && c <= 'z') ||
        ('A' <= c && c <= 'Z');
}

static bool isNum(const char c) {
    return '0' <= c && c <= '9';
}

static bool isAlNum(const char c) {
    return isAlpha(c) || isNum(c);
}


char Lexer::peek() const {
    return this->source[head];
}

char Lexer::advance() {
    return source[head++]; // advance after grabbing char
}

bool Lexer::isAtEnd() const {
    return head >= source.length();
}

char Lexer::peekNext() const {
    if (isAtEnd()) return '\0';
    return source[head + 1];
}

// Token creators

// generic/strings
Token Lexer::makeToken(const TokenType type, const char* data, u16 beginningLine) const {
    return (Token) {type, beginningLine, base, {.name = data}};
}

// for identifier tokens
Token Lexer::makeToken(const TokenType type, const char* data) const {
    return makeToken(type, data, line);
}

// for number tokens
Token Lexer::makeToken(const TokenType type, double number) const {
    return (Token) {type, line, base, {.number = number}};
}

Token Lexer::noDataToken(const TokenType type) const {
    return makeToken(type, nullptr, line);
}

Token Lexer::errorToken(const char* message) const {
    return (Token) {ERROR, line, base, {message}};
}

Token Lexer::errorToken(const char* message, u16 beginningLine) const {
    return (Token) {ERROR, beginningLine, base, {message}};
}

// internals

Token Lexer::number() {
    while (isNum(peek())) advance();

    if (peek() == '.') {
        advance();
        while (isNum(peek())) advance();
    }

    const double value = strtod(source.c_str() + base, nullptr);

    return makeToken(NUMBER, value);
}

Token Lexer::string() {
    const u16 start = line;

    while (peek() != '"' && !isAtEnd())
        if (advance() == '\n') line++;

    if (isAtEnd()) return errorToken(static_cast<const char *>("Unterminated string"), start);

    advance();

    const u32 beginning = base + 1; // eat beginning "

    u32 size = head - beginning;

    char *data = (char*) malloc(sizeof(char) * size);
    memcpy(data, source.c_str() + beginning, size - 1);
    data[size - 1] = '\0';

    return makeToken(STRING, data, start);

}

Token Lexer::checkKeyword(const u16 start, const char *remaining, TokenType type) const {
    const u16 len = strlen(remaining);
    if (len + start == head - base && memcmp(remaining, &source.c_str()[base + start], len) == 0) // use c_str to ensure we get the correct memory location
        return noDataToken(type);

    return noDataToken(IDENTIFIER);
}

Token Lexer::keyword() const {
    const u32 length = head - base;
    switch (source[base]) {
        case 'e': if (length > 1) {
            switch (source[base + 1]) {
                case 'l': return checkKeyword(2, "se", ELSE);
                case 'x': return checkKeyword(2, "tern", EXTERN);
                default:
                    break;
            }
        }
            break;

        case 'f':
            if (length > 1) {
                switch (source[base + 1]) {
                    case 'a': return checkKeyword(2, "lse", FALSE);
                    case 'o': return checkKeyword(2, "r", FOR);
                    case '3': return checkKeyword(2, "2", F32);
                    case '6': return checkKeyword(2, "4", F64);
                    default:
                        break;
                }
            }
            break;
        case 'i':
            if (length > 1) {
                switch (source[base + 1]) {
                    case '8': return checkKeyword(2, "", I8);
                    case '1': return checkKeyword(2, "6", I16);
                    case '3': return checkKeyword(2, "2", I32);
                    case '6': return checkKeyword(2, "4", I64);
                    case 'f': return checkKeyword(2, "", IF);
                    default:
                        break;
                }
            }
            break;

        case 'p': return checkKeyword(1, "rint", PRINT);

        case 'r': return checkKeyword(1, "eturn", RETURN);

        case 't': return checkKeyword(1, "rue", TRUE);

        case 'u':
            if (length > 1) {
                switch (source[base + 1]) {
                    case '8': return checkKeyword(2, "", U8);
                    case '1': return checkKeyword(2, "6", U16);
                    case '3': return checkKeyword(2, "2", U32);
                    case '6': return checkKeyword(2, "4", U64);
                    default:
                        break;
                }
            }
            break;

        case 'v': return checkKeyword(1, "oid", VOID);
        case 'w': return checkKeyword(1, "hile", WHILE);
        default:
            break;
    }
    return noDataToken(IDENTIFIER);
}

Token Lexer::identifier() {

    while (true) {
        const char c = peek();
        if (!(isAlNum(c) || c == '_')) break;

        advance();
    }

    // keyword check
    const Token token = keyword();
    if (token.type != IDENTIFIER) return token;


    // is identifier

    const size_t size = head - base + 1;

    char *data = (char*) malloc(sizeof(char) * size);
    memcpy(data, source.c_str() + base, size - 1);
    data[size - 1] = '\0';
    return makeToken(IDENTIFIER, data);

}


Token Lexer::scanToken() {

    skipWhitespace();
    base = head;

    if (isAtEnd()) return noDataToken(END_OF_FILE);

    const char c = advance();

    switch (c) {

        // single-character tokens
        case ';': return noDataToken(SEMICOLON);
        case '(': return noDataToken(LEFT_PAREN);
        case ')': return noDataToken(RIGHT_PAREN);
        case '{': return noDataToken(LEFT_BRACE);
        case '}': return noDataToken(RIGHT_BRACE);
        case '[': return noDataToken(LEFT_BRACKET);
        case ']': return noDataToken(RIGHT_BRACKET);

        case ',': return noDataToken(COMMA);
        case '.': return noDataToken(DOT);

        case '~': return noDataToken(TILDE);

        // possibly two-character-tokens
        case '+':
            switch (peek()) {
                case '=': advance(); return noDataToken(PLUS_EQUALS);
                case '+': advance(); return noDataToken(PLUS_PLUS);
                default:
                    return noDataToken(PLUS);
            }
        case '-':
            switch (peek()) {
            case '=': advance(); return noDataToken(MINUS_EQUALS);
            case '-': advance(); return noDataToken(MINUS_MINUS);
            default:
                    return noDataToken(MINUS);
            }
        case '*':
            if (peek() == '=') {
                advance();
                return noDataToken(STAR_EQUALS);
            }
            return noDataToken(STAR);
        case '/':
            if (peek() == '=') {
                advance();
                return noDataToken(SLASH_EQUALS);
            }
            return noDataToken(SLASH);

        case '=':
            if (peek() == '=') {
                advance();
                return noDataToken(EQUALS_EQUALS);
            }
            return noDataToken(EQUALS);
        case '!':
            if (peek() == '=') {
                advance();
                return noDataToken(BANG_EQUALS);
            }
            return noDataToken(BANG);
        case '>':
            if (peek() == '=') {
                advance();
                return noDataToken(MORE_EQUALS);
            }
            return noDataToken(MORE);
        case '<':
            if (peek() == '=') {
                advance();
                return noDataToken(LESS_EQUALS);
            }
            return noDataToken(LESS);

        case '|':
            switch (peek()) {
                case '=': advance(); return noDataToken(PIPE_EQUALS);
                case '|': advance(); return noDataToken(PIPE_PIPE);

                default: return noDataToken(PIPE);
            }

        case '&':
            switch (peek()) {
                case '=': advance(); return noDataToken(AMP_EQUALS);
                case '&': advance(); return noDataToken(AMP_AMP);

                default: return noDataToken(AMP);
            }

        default:
            break;
    }

    // multi-character tokens

    if (isNum(c)) return number();
    if (c == '"') return string();

    if (isAlpha(c) || c == '_') return identifier();

    return errorToken((char*) "Unexpected character.");
}

/**
 * @return Whether or not this was a comment (true = comment, false = no comment)
 */
bool Lexer::comment() {

    switch (peekNext()) {
        case '/': //line comment
            while (peek() != '\n') advance();
            return true;
        case '*': //block comment
            while (!(peek() == '*' && peekNext() == '/')) {

                if (peek() == '\n') line++;
                if (isAtEnd()) throw Lexing::ScanException("Unterminated comment");
                advance();
            }

            advance(); // consume trailing */
            advance();
            return true;
        default:
            return false;
    }
}


void Lexer::skipWhitespace() {
    while (true) {
        const char c = peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r': {
                advance();
                break;
            }
            case '\n': {
                line++;
                advance();
                break;
            }
            case '/':
                if (!comment()) return;
                break;
            default: return;

        }
    }
}


std::vector<Token> Lexing::scan(const std::string& input) {

    std::vector<Token> tokens;
    Lexer lexer(input);

    Token token = {.type = ERROR, .line = 0, .position = 0, .data = {.name = nullptr}};

    lexer.skipWhitespace();

    while (token.type != END_OF_FILE) {
        token = lexer.scanToken();
        tokens.push_back(token);
    }

    return tokens;

}
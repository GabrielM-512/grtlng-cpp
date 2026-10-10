#include "parsing.h"
#include "../compiler.h"

using namespace Parsing;

Stmt::Stmt* Parser::statement() {
    if (match(Lexing::PRINT)) return printStatement();
    if (match(Lexing::IF)) return ifStatement();
    if (match(Lexing::WHILE)) return whileStatement();
    if (match(Lexing::FOR)) return forStatement();
    if (match(Lexing::LEFT_BRACE)) return blockStatement();
    if (match(Lexing::RETURN)) return returnStatement();
    if (match(Lexing::BREAK)) return breakStatement();
    if (match(Lexing::CONTINUE)) return continueStatement();
    if (match(Lexing::SWITCH)) return switchStatement();

    if (checkTypeIdent()) throw errorAtCurrent("Unexpected type identifier", "Variable declarations are not allowed in this context");

    return expressionStatement();
}

Stmt::Stmt* Parser::declaration() {
    if (matchTypeIdent()) return localDeclarationStatement();
    return statement();
}

Stmt::Print *Parser::printStatement() {
    Lexing::Token print = previous;

    Expr::Expr* expr = nullptr;
    if (check(Lexing::SEMICOLON)) {
        fatalErrorAtCurrent("Expected expression");
    } else {
        expr = expression();
    }

    consume(Lexing::SEMICOLON);
    return new Stmt::Print(print, expr);
}

Stmt::Expression *Parser::expressionStatement() {
    Expr::Expr* expr = expression();
    consume(Lexing::SEMICOLON);
    return new Stmt::Expression(expr->token, expr);
}

Stmt::VariableDeclaration *Parser::variableDeclaration(Lexing::Token dataType, const Lexing::Token &name) {
    Expr::Expr* value = nullptr;

    if (match(Lexing::EQUALS)) {
        value = expression();
    }

    consume(Lexing::SEMICOLON, " after variable declaration");

    return new Stmt::VariableDeclaration(dataType, dataType.type, name, value);
}

Stmt::VariableDeclaration *Parser::localDeclarationStatement() {
    Lexing::Token dataType = previous;
    consume(Lexing::IDENTIFIER, " after data type");

    if (match(Lexing::LEFT_PAREN)) {
        throw error("Unexpected '(' in local variable declaration", "Function declarations are only permitted in the global scope");
    }

    const Lexing::Token name = previous;

    return variableDeclaration(dataType, name);
}

Stmt::If *Parser::ifStatement() {
    Lexing::Token if_ = previous;

    consume(Lexing::LEFT_PAREN, " after \"if\"");
    Expr::Expr* condition = expression();
    consume(Lexing::RIGHT_PAREN, " after if condition");

    Stmt::Stmt* thenBranch = statement();
    Stmt::Stmt* elseBranch = nullptr;

    if (match(Lexing::ELSE)) elseBranch = statement();

    return new Stmt::If(if_, condition, thenBranch, elseBranch);
}

Stmt::While *Parser::whileStatement() {
    Lexing::Token while_ = previous;

    consume(Lexing::LEFT_PAREN, " after \"while\"");
    Expr::Expr* condition = expression();
    consume(Lexing::RIGHT_PAREN, " after while condition");

    loopCount++;
    Stmt::Stmt* body = statement();
    loopCount--;

    return new Stmt::While(while_, condition, body);
}

Stmt::Stmt* Parser::forStatement() {
    Lexing::Token for_ = previous;

    consume(Lexing::LEFT_PAREN, " after \"while\"");
    Stmt::Stmt* initialiser = nullptr;

    if (!match(Lexing::SEMICOLON)) {
        // initialiser
        if (matchTypeIdent()) initialiser = localDeclarationStatement();
        else initialiser = expressionStatement();
    }

    Expr::Expr* condition;

    if(!check(Lexing::SEMICOLON)) {
        // condition
        condition = expression();
    } else {
        // always truthy
        condition = new Expr::Number(peek(), 1);
    }

    consume(Lexing::SEMICOLON, " after condition");


    Expr::Expr* incrementer = nullptr;

    if (!check(Lexing::RIGHT_PAREN)) {
        incrementer = expression();
    }

    consume(Lexing::RIGHT_PAREN, " after incrementer clause");

    loopCount++;
    Stmt::Stmt* body = statement();
    loopCount--;

    return new Stmt::For(for_, initialiser, condition, incrementer, body);
}

Stmt::Block *Parser::blockStatement() {
    Lexing::Token brace = previous;
    std::vector<Stmt::Stmt*> contents;
    while (!match(Lexing::RIGHT_BRACE)) {
        if (check(Lexing::END_OF_FILE)) {
            errorAtCurrent("Unterminated block");
            break;
        }
        Stmt::Stmt* stmt = declaration();
        contents.push_back(stmt);
    }

    return new Stmt::Block(brace, contents);
}

Stmt::Return *Parser::returnStatement() {
    Lexing::Token return_ = previous;

    Expr::Expr* value = nullptr;

    if (!check(Lexing::SEMICOLON))
        value = expression();

    consume(Lexing::SEMICOLON, " after return value");
    return new Stmt::Return(return_, value);
}

Stmt::Break *Parser::breakStatement() {
    Lexing::Token break_ = previous;

    if (!hasLoop() && !hasSwitch()) {
        errorAtCurrent("Used \"break\" outside of a loop or switch statement");
    }

    consume(Lexing::SEMICOLON, " after \"break\"");

    return new Stmt::Break(break_);

}

Stmt::Continue *Parser::continueStatement() {
    Lexing::Token continue_ = previous;

    if (!hasLoop()) {
        errorAtCurrent("Used \"continue\" outside of a loop");
    }

    consume(Lexing::SEMICOLON, " after \"continue\"");

    return new Stmt::Continue(continue_);
}

Stmt::Case Parser::switchCase(std::optional<Expr::Expr *> value, Lexing::Token exprToken) {
    std::vector<Stmt::Stmt*> stmts;

    while (!check(Lexing::CASE) && !check(Lexing::DEFAULT) && !check(Lexing::END_OF_FILE) && !check(Lexing::RIGHT_BRACE)) {
        stmts.push_back(statement());
    }

    return {.value = value, .valueToken = exprToken, .content = stmts};
}

Stmt::Switch *Parser::switchStatement() {
    Lexing::Token switch_ = previous;

    switchCount++;

    consume(Lexing::LEFT_PAREN, " after switch");

    Expr::Expr* value = expression();

    consume(Lexing::RIGHT_PAREN, " after switch value");
    consume(Lexing::LEFT_BRACE, " before switch cases");
    std::vector<Stmt::Case> cases;

    while (true) {
        if (check(Lexing::END_OF_FILE)) throw Compiler::CompileError("Unterminated switch statement body", peek());
        if (match(Lexing::RIGHT_BRACE)) break;

        if (match(Lexing::CASE)) {
            Lexing::Token exprStart = peek();
            Expr::Expr *condition = expression();

            consume(Lexing::COLON, " after switch value");

            cases.push_back(switchCase(condition, exprStart));
        } else if (match(Lexing::DEFAULT)) {
            for (const Stmt::Case& case_ : cases) {
                if (!case_.value.has_value()) throw Compiler::CompileError("Redefined default case", previous);
            }

            consume(Lexing::COLON, " after default");

            cases.push_back(switchCase(std::nullopt, fakeToken()));
        } else {
            switchCount--;
            throw Compiler::CompileError("Expected \"case\" or \"default\", got " + peek().toString() + "instead", peek());
        }
    }

    switchCount--;

    return new Stmt::Switch(switch_, value, cases);
}

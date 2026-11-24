#include "bytecode.h"

#include <cmath>
#include <iostream>
#include <ranges>
#include <unordered_map>

#include "parsing/parsing.h"
#include "../util/fileIO.h"

using namespace Bytecode;

bool is_i8(double number) {
    double fraction;
    return std::modf(number, &fraction) == 0 && INT8_MIN <= number && INT8_MAX >= number;
}


std::vector<u8>& BytecodeCompiler::currentChunk() {
    if (inInit) return init;
    return currentFunction;
}

void BytecodeCompiler::emitByte(u8 byte) {
    currentChunk().push_back(byte);
}

void BytecodeCompiler::emitBytes(u8 byte1, u8 byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void BytecodeCompiler::emitConstant(Value::Value value) {
    u32 index = addConstant(value);

    if (index > UINT8_MAX) throw CompileError("Too many constants in a chunk");

    emitBytes(LOAD_CONSTANT, (u8) index);
}

void BytecodeCompiler::emitConstant(f64 value) {
    emitConstant(VALUE_NUM(value));
}

void BytecodeCompiler::emitPops(u8 popCount) {
    if (popCount == 0) return;
    if (popCount == 1) emitByte(POP);
    else emitBytes(POP_N, popCount);
}

u32 BytecodeCompiler::addConstant(Value::Value value) {

    for (u64 i = 0; i < program.constants.size(); i++) {
        if (Value::equality(value, program.constants.at(i))) {
            return i;
        }
    }

    program.constants.push_back(value);
    return program.constants.size() - 1;
}

u16 BytecodeCompiler::createGlobal(const std::string& name) {
    u16 index = globals.size();

    if (index > UINT8_MAX) throw CompileError("Too many global variables [TODO]");

    globals.insert({name, index});

    return index;
}

void BytecodeCompiler::createLocal(const std::string& name) {
    locals.push_back((Local) {.name = name, .depth = scopeDepth});
}

/**
 * @param name name of the variable to resolve
 * @return the globals array list index of the variable if found, -1 otherwise
 */
i32 BytecodeCompiler::resolveGlobal(const std::string& name) {
    auto var = globals.find(name);

    if (var == globals.end()) {
        return -1;
    }

    return var->second;
}

/**
 * @param name name of the variable to resolve
 * @return The stack slot index of the variable if found, -1 otherwise
 */
i16 BytecodeCompiler::resolveLocal(const std::string& name) const {

    for (i16 i = locals.size() - 1; i >= 0; i--) {
        if (locals.at(i).name == name) return i;
    }

    return -1;
}

void BytecodeCompiler::beginScope() {
    scopeDepth++;
}

void BytecodeCompiler::endScope() {
    // remove current scoped vars
    u8 popCount = 0;

    while (!locals.empty() && locals.back().depth == scopeDepth) {
        locals.pop_back();
        popCount++;
    }

    emitPops(popCount);

    scopeDepth--;

}

u32 BytecodeCompiler::countCurrentScopeVars() const {
    if (locals.empty()) return 0;

    u32 count = 0;

    for (i64 i = locals.size() - 1; i >= 0; i--) {
        if (locals.at(i).depth != scopeDepth) break;

        count++;
    }

    return count;
}

void BytecodeCompiler::loadNamedVariable(const std::string& name) {
    i32 index = resolveLocal(name);
    Operation op = LOAD_LOCAL;

    if (index == -1) {
        index = resolveGlobal(name);
        op = LOAD_GLOBAL;
    }

    if (index == -1) throw CompileError("Unknown Variable " + name);

    emitBytes(op, (u8) index);
}

u64 BytecodeCompiler::emitJump(Operation operation) {
    emitByte(operation);
    emitBytes(0xff, 0xff);

    return currentChunk().size() - 2;
}

void BytecodeCompiler::patchJump(u64 jumpLocation) {
    // -2 to adjust for the bytecode for the jump offset
    u64 difference = currentChunk().size() - jumpLocation - 2;

    if (difference > UINT16_MAX) throw CompileError("Too much code to jump over");

    u16 jumpValue = difference;

    currentChunk().at(jumpLocation) = (jumpValue >> 8) & 0xff;
    currentChunk().at(jumpLocation + 1) = jumpValue & 0xff;
}

void BytecodeCompiler::emitLoop(u64 loopStart) {
    emitByte(LOOP);

    u64 distance = currentChunk().size() - loopStart + 2;
    if (distance > UINT16_MAX) throw CompileError("Loop body too large");

    u16 offset = distance;

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}


void BytecodeCompiler::compileFunction(const Stmt::Function* function) {
    // use a pointer instead of a reference so that the original doesn't get destoryed taking its child nodes with it

    inInit = false;

    beginScope();

    for (Stmt::VariableDeclaration* param: function->params) {
        createLocal(param->name.data.name);
    }

    for (Stmt::Stmt* stmt : function->body->statements) {
        compileStatement(stmt);
    }

    endScope();

    auto* compiledFunction = new Value::Function(currentChunk(), function->name.data.name, (u8) function->params.size());

    currentFunction.clear();

    inInit = true;

    emitConstant(VALUE_FUNCTION(compiledFunction));
    emitBytes(SET_GLOBAL, resolveGlobal(function->name.data.name));
    emitByte(POP);

    if (function->params.size() > UINT8_MAX) {
        std::string message = "Error: Function \"" + std::string(function->name.data.name) + "\" takes " + std::to_string(function->params.size()) + " parameters, maximum allowed is 255";
        errors.push_back(message);
    }
}

void BytecodeCompiler::compileExpression(Expr::Expr* expr) {
    expr->accept(this);
}

void BytecodeCompiler::compileStatement(Stmt::Stmt* stmt) {
    stmt->accept(this);
}

void BytecodeCompiler::declareNatives() {
    auto natives = Value::nativeFnDefinitions();
    for (const Value::NativeFn& fn : natives) {
        auto current = new Value::NativeFn(fn);
        createGlobal(fn.name);

        emitConstant(VALUE_NATIVE(current));
        emitBytes(SET_GLOBAL, resolveGlobal(fn.name));
        emitByte(POP);
    }
}

void BytecodeCompiler::startBreakable(BreakType type) {
    breakables.push_back({.type = type, .localsSize = static_cast<i32>(locals.size()), .breaks = {}, .continues = {}});
}

void BytecodeCompiler::startLoop() {
    startBreakable(BREAK_LOOP);
}

void BytecodeCompiler::startSwitch() {
    startBreakable(BREAK_SWITCH);
}

void BytecodeCompiler::endLoop() {
    breakables.pop_back();
}

BreakStop& BytecodeCompiler::findLoop() {
    for (auto & breakable : std::views::reverse(breakables)) {
        if (breakable.type == BREAK_LOOP) return breakable;
    }

    // unreachable if used properly

    throw CompileError("No loop");
}

BreakStop& BytecodeCompiler::findSwitch() {
    for (auto & breakable : std::views::reverse(breakables)) {
        if (breakable.type == BREAK_SWITCH) return breakable;
    }

    // unreachable if used properly

    throw CompileError("No switch");
}

void BytecodeCompiler::compileSwitchCase(const Stmt::Case& case_) {
    for (Stmt::Stmt* stmt : case_.content) {
        compileStatement(stmt);
    }
}

Program BytecodeCompiler::compile(const Compiler::SyntaxTree& ast) {
    std::vector<Stmt::Function*> funcs;
    for (Stmt::Stmt* current : ast.tree) {
        if (auto func = dynamic_cast<Stmt::Function*> (current)) {
            createGlobal(func->name.data.name);
            if (func->body->statements.empty() || !dynamic_cast<Stmt::Return*>(func->body->statements.back())) {
                // allocate a new node every time so we can properly free them all after compilation
                auto zeroReturn = new Stmt::Return(Parsing::Parser::fakeToken(), new Expr::Number(Parsing::Parser::fakeToken(), 0));
                func->body->statements.push_back(zeroReturn);
            }

            funcs.push_back(func);
        } else {
            auto var = dynamic_cast<Stmt::VariableDeclaration*>(current);

            createGlobal(var->name.data.name);
            if (var->value != nullptr) {
                compileExpression(var->value);

                emitBytes(SET_GLOBAL, resolveGlobal(var->name.data.name));
                emitByte(POP);
            }
        }
    }

    declareNatives();

    for (Stmt::Function* func : funcs) {
        compileFunction(func);
    }

    if (!errors.empty()) {
        std::string messages;

        for (const std::string& message : errors) {
            messages += message + "\n";
        }

        throw CompileError(messages);
    }

    loadNamedVariable("main");
    emitBytes(CALL, 0);
    emitByte(EXIT);

    program.globalCount = globals.size();
    program.code = init;

    return program;
}


ExprVisitResults BytecodeCompiler::visitAssignExpr(Expr::Assign *expr)  {

    compileExpression(expr->value);

    i32 index = resolveGlobal(expr->name.data.name);
    Operation op = SET_GLOBAL;

    if (index == -1) {
        index = resolveLocal(expr->name.data.name);
        op = SET_LOCAL;
    }

    emitBytes(op, (u8) index);

    return std::monostate();
}

ExprVisitResults BytecodeCompiler::visitBinaryExpr(Expr::Binary *expr)  {
    compileExpression(expr->left);
    compileExpression(expr->right);


    switch (expr->operatorType) {
        case Lexing::PLUS: emitByte(ADD); break;
        case Lexing::MINUS: emitByte(SUBTRACT); break;
        case Lexing::STAR: emitByte(MULTIPLY); break;
        case Lexing::SLASH: emitByte(DIVIDE); break;

        case Lexing::EQUALS_EQUALS: emitByte(EQUALS); break;
        case Lexing::BANG_EQUALS: emitBytes(EQUALS, NOT); break;

        case Lexing::LESS: emitByte(LESS); break;
        case Lexing::LESS_EQUALS: emitByte(LESS_EQUALS); break;
        case Lexing::MORE: emitByte(MORE); break;
        case Lexing::MORE_EQUALS: emitByte(MORE_EQUALS); break;

        default:
            throw CompileError("Invalid binary token " + Lexing::Token::toString(expr->operatorType));
    }

    return std::monostate();
}

ExprVisitResults BytecodeCompiler::visitCallExpr(Expr::Call *expr)  {
    compileExpression(expr->callee);

    for (Expr::Expr* arg : expr->args) {
        compileExpression(arg);
    }

    u8 argCount = (u8) expr->args.size();
    emitBytes(CALL, argCount);

    return std::monostate();
}

ExprVisitResults BytecodeCompiler::visitConditionalExpr(Expr::Conditional *expr)  {
    compileExpression(expr->condition);
    u64 elseJump = emitJump(JUMP_FALSE);

    emitByte(POP); // pop condition (truthy path)

    compileExpression(expr->thenBranch);

    u64 exitJump = emitJump(JUMP);

    patchJump(elseJump); // begin falsy path

    emitByte(POP); // pop condition

    compileExpression(expr->elseBranch);

    patchJump(exitJump);

    return std::monostate();
}

ExprVisitResults BytecodeCompiler::visitIdentifierExpr(Expr::Identifier *expr)  {
    loadNamedVariable(expr->token.data.name);
    return std::monostate();
}

ExprVisitResults BytecodeCompiler::visitLogicalExpr(Expr::Logical *expr)  {
    Operation jumpType;
    Operation first, second;

    if (expr->operatorType == Lexing::AMP_AMP) {
        jumpType = JUMP_FALSE;
        first = TRUE;
        second = FALSE;
    } else if (expr->operatorType == Lexing::PIPE_PIPE) {
        jumpType = JUMP_TRUE;
        first = FALSE;
        second = TRUE;
    } else throw CompileError("Invalid logical expression operator: " + Lexing::Token::toString(expr->operatorType));

    compileExpression(expr->left);
    u64 aJump = emitJump(jumpType);

    emitByte(POP);
    compileExpression(expr->right);
    u64 bJump = emitJump(jumpType);

    emitBytes(POP, first);
    u64 firstJump = emitJump(JUMP);

    patchJump(aJump);
    patchJump(bJump);

    emitBytes(POP, second);

    patchJump(firstJump);

    return std::monostate();
}

ExprVisitResults BytecodeCompiler::visitNumberExpr(Expr::Number *expr)  {

    if (expr->value == 0) {
        emitByte(FALSE);
        return std::monostate();
    }

    if (expr->value == 1) {
        emitByte(TRUE);
        return std::monostate();
    }

    if (is_i8(expr->value)) {
        // use LOAD_I8

        i8 value = static_cast<i8>(expr->value);

        u8 number = *((u8*) &value);

        emitBytes(LOAD_I8, number);

        return std::monostate();
    }

    // use LOAD_CONSTANT

    emitConstant(expr->value);

    return std::monostate();
}

ExprVisitResults BytecodeCompiler::visitUnaryExpr(Expr::Unary *expr)  {
    compileExpression(expr->right);

    Operation op;
    switch (expr->operatorType) {
        case Lexing::MINUS: op = NEGATE; break;
        case Lexing::BANG: op = NOT; break;

        default: throw CompileError("Unknown unary type"); // unreachable
    }

    emitByte(op);

    return std::monostate();
}



StmtVisitResults BytecodeCompiler::visitBlockStmt(Stmt::Block *stmt)  {
    beginScope();

    for (Stmt::Stmt* current : stmt->statements) {
        compileStatement(current);
    }

    endScope();

    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitBreakStmt(Stmt::Break *)  {

    u8 popCount = locals.size() - breakables.back().localsSize;
    emitPops(popCount);

    u64 jump = emitJump(JUMP);
    breakables.back().breaks.push_back(jump);

    return std::monostate();
}


StmtVisitResults BytecodeCompiler::visitContinueStmt(Stmt::Continue *)  {

    u8 popCount = locals.size() - breakables.back().localsSize;
    emitPops(popCount);

    u64 jump = emitJump(JUMP);
    findLoop().continues.push_back(jump);

    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitExpressionStmt(Stmt::Expression *stmt)  {
    compileExpression(stmt->expression);
    emitByte(POP);
    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitForStmt(Stmt::For *stmt)  {

    beginScope();

    if (stmt->initialiser != nullptr) compileStatement(stmt->initialiser);

    u64 loopStart = currentChunk().size();

    compileExpression(stmt->condition);
    u64 exitJump = emitJump(JUMP_FALSE);

    emitByte(POP); // pop the condition true

    startLoop();

    compileStatement(stmt->body);

    for (u64 cont : breakables.back().continues) {
        patchJump(cont);
    }

    if (stmt->incrementer != nullptr) {
        compileExpression(stmt->incrementer);
        emitByte(POP);
    }

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(POP); // pop the condition if exited loop

    for (u64 brk : breakables.back().breaks) {
        patchJump(brk);
    }

    endLoop();

    endScope();

    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitFunctionStmt(Stmt::Function *stmt)  {
    compileFunction(stmt);

    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitIfStmt(Stmt::If *stmt)  {
    compileExpression(stmt->condition);

    u64 elseJump = emitJump(JUMP_FALSE);
    emitByte(POP); // pop condition (truthy branch)

    compileStatement(stmt->thenBranch);

    u64 thenJump = emitJump(JUMP);

    patchJump(elseJump);

    emitByte(POP); // pop condition (falsy path)
    if (stmt->elseBranch != nullptr) compileStatement(stmt->elseBranch);

    patchJump(thenJump);

    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitPrintStmt(Stmt::Print *stmt)  {
    compileExpression(stmt->expression);
    emitByte(PRINT);

    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitReturnStmt(Stmt::Return *stmt)  {
    if (stmt->value != nullptr) compileExpression(stmt->value);
    else emitByte(FALSE);

    emitByte(RETURN);

    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitVariableDeclarationStmt(Stmt::VariableDeclaration *stmt)  {
    if (stmt->value != nullptr) compileExpression(stmt->value);
    else emitByte(FALSE);

    createLocal(stmt->name.data.name);

    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitWhileStmt(Stmt::While *stmt)  {
    u64 loopStart = currentChunk().size();

    compileExpression(stmt->condition);

    u64 exitJump = emitJump(JUMP_FALSE);

    emitByte(POP); // pop the condition true

    startLoop();

    compileStatement(stmt->body);

    for (u64 cont : breakables.back().continues) {
        patchJump(cont);
    }

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(POP); // pop the condition if exited loop

    for (u64 brk : breakables.back().breaks) {
        patchJump(brk);
    }

    endLoop();

    return std::monostate();
}

StmtVisitResults BytecodeCompiler::visitSwitchStmt(Stmt::Switch *stmt)  {
    beginScope();

    compileExpression(stmt->condition);
    createLocal("");

    startSwitch();

    std::vector<u64> entryJumps;
    bool hasDefault = false;

    for (const Stmt::Case& currentCase : stmt->cases) {
        if (!currentCase.value.has_value()) {
            hasDefault = true;
            continue;
        }


        // compare values
        compileExpression(currentCase.value.value());
        loadNamedVariable("");
        emitByte(EQUALS);

        // jump, get rid of condition if falsey
        entryJumps.push_back(emitJump(JUMP_TRUE));
        emitByte(POP);
    }

    const u64 defaultJump = emitJump(JUMP);

    {
        u64 currentIndex = 0;
        for (const Stmt::Case& currentCase : stmt->cases) {
            if (currentCase.value.has_value()) {
                patchJump(entryJumps.at(currentIndex++));
                emitByte(POP); // pop the condition
            } else {
                patchJump(defaultJump);
            }

            compileSwitchCase(currentCase);
        }
    }

    if (!hasDefault) patchJump(defaultJump);

    for (u64 exitJump : findSwitch().breaks) {
        patchJump(exitJump);
    }

    endScope();

    breakables.pop_back();

    return std::monostate();
}
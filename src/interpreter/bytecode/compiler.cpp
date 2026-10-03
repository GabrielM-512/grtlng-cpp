#include "compiler.h"

#ifndef INTERPRETER_AST

#include <cmath>
#include <iostream>
#include <unordered_map>

using namespace BytecodeCompilation;

struct Local {
    std::string name;
    i32 depth;
};

class BytecodeCompiler : public Stmt::StmtVisitor, public Expr::ExprVisitor{
    Bytecode::Program program {};
    std::unordered_map<std::string, u16> globals;
    std::vector<Local> locals;

    std::vector<u8> currentFunction;
    std::vector<u8> init;

    bool inInit = true;
    std::vector<std::string> errors;

    i32 scopeDepth = 0;

    std::vector<u8>& currentChunk() {
        if (inInit) return init;
        return currentFunction;
    }

    void emitByte(u8 byte) {
        currentChunk().push_back(byte);
    }

    void emitBytes(u8 byte1, u8 byte2) {
        emitByte(byte1);
        emitByte(byte2);
    }

    void emitConstant(Value::Value value) {
        u32 index = addConstant(value);

        if (index > UINT8_MAX) throw CompileError("Too many constants in a chunk");

        emitBytes(Bytecode::LOAD_CONSTANT, (u8) index);
    }

    void emitConstant(f64 value) {
        emitConstant(VALUE_NUM(value));
    }

    u32 addConstant(Value::Value value) {

        for (u64 i = 0; i < program.constants.size(); i++) {
            if (Value::equality(value, program.constants.at(i))) {
                return i;
            }
        }

        program.constants.push_back(value);
        return program.constants.size() - 1;
    }

    u16 createGlobal(const std::string& name) {
        u16 index = globals.size();

        if (index > UINT8_MAX) throw CompileError("Too many global variables [TODO]");

        globals.insert({name, index});

        return index;
    }

    void createLocal(const std::string& name) {
        locals.push_back((Local) {.name = name, .depth = scopeDepth});
    }

    /**
     * @param name name of the variable to resolve
     * @return the globals array list index of the variable if found, -1 otherwise
     */
    i32 resolveGlobal(const std::string& name) {
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
    i16 resolveLocal(const std::string& name) const {

        for (i16 i = locals.size() - 1; i >= 0; i--) {
            if (locals.at(i).name == name) return i;
        }

        return -1;
    }

    void beginScope() {
        scopeDepth++;
    }

    void endScope() {
        // remove current scoped vars
        while (!locals.empty() && locals.back().depth == scopeDepth) {
            locals.pop_back();
        }

        scopeDepth--;

    }

    u32 countCurrentScopeVars() const {
        if (locals.empty()) return 0;

        u32 count = 0;

        for (i64 i = locals.size() - 1; i >= 0; i--) {
            if (locals.at(i).depth != scopeDepth) break;

            count++;
        }

        return count;
    }

    void loadNamedVariable(const std::string& name) {
        i32 index = resolveLocal(name);
        Bytecode::Operation op = Bytecode::LOAD_LOCAL;

        if (index == -1) {
            index = resolveGlobal(name);
            op = Bytecode::LOAD_GLOBAL;
        }

        if (index == -1) throw CompileError("Unknown Variable " + name);

        emitBytes(op, (u8) index);
    }

    u64 emitJump(Bytecode::Operation operation) {
        emitByte(operation);
        emitBytes(0xff, 0xff);

        return currentChunk().size() - 2;
    }

    void patchJump(u64 jumpLocation) {
        // -2 to adjust for the bytecode for the jump offset
        u64 difference = currentChunk().size() - jumpLocation - 2;

        if (difference > UINT16_MAX) throw CompileError("Too much code to jump over");

        u16 jumpValue = difference;

        currentChunk().at(jumpLocation) = (jumpValue >> 8) & 0xff;
        currentChunk().at(jumpLocation + 1) = jumpValue & 0xff;
    }

    void emitLoop(u64 loopStart) {
        emitByte(Bytecode::LOOP);

        u64 distance = currentChunk().size() - loopStart + 2;
        if (distance > UINT16_MAX) throw CompileError("Loop body too large");

        u16 offset = distance;

        emitByte((offset >> 8) & 0xff);
        emitByte(offset & 0xff);
    }


    void compileFunction(const Stmt::Function& function) {

        inInit = false;

        beginScope();

        for (Stmt::VariableDeclaration* param: function.params) {
            createLocal(param->name.data.name);
        }

        for (Stmt::Stmt* stmt : function.body->statements) {
            compileStmt(stmt);
        }

        endScope();

        auto* compiledFunction = new Value::Function(currentChunk(), function.name.data.name, (u8) function.params.size());

        currentFunction.clear();

        inInit = true;

        emitConstant(VALUE_FUNCTION(compiledFunction));
        emitBytes(Bytecode::SET_GLOBAL, resolveGlobal(function.name.data.name));
        emitByte(Bytecode::POP);

        if (function.params.size() > UINT8_MAX) {
            std::string message = "Error: Function \"" + std::string(function.name.data.name) + "\" takes " + std::to_string(function.params.size()) + " parameters, maximum allowed is 255";
            errors.push_back(message);
        }
    }

    void compileExpression(Expr::Expr* expr) {
        expr->accept(this);
    }

    void compileStmt(Stmt::Stmt* stmt) {
        stmt->accept(this);
    }

    void declareNatives() {
        auto natives = Value::nativeFnDefinitions();
        for (const Value::NativeFn& fn : natives) {
            auto current = new Value::NativeFn(fn);
            createGlobal(fn.name);

            emitConstant(VALUE_NATIVE(current));
            emitBytes(Bytecode::SET_GLOBAL, resolveGlobal(fn.name));
            emitByte(Bytecode::POP);
        }
    }

public:
    Bytecode::Program compile(const Compiler::CompileResult& ast) {
        std::vector<Stmt::Function*> funcs;
        for (Stmt::Stmt* current : ast.tree) {
            if (auto func = dynamic_cast<Stmt::Function*> (current)) {
                createGlobal(func->name.data.name);
                if (func->body->statements.empty() || !dynamic_cast<Stmt::Return*>(func->body->statements.back())) {

                    static Expr::Number zeroExpr(0.0);
                    static Stmt::Return zeroReturn(&zeroExpr);

                    func->body->statements.push_back(&zeroReturn);
                }

                funcs.push_back(func);
            } else {
                auto var = dynamic_cast<Stmt::VariableDeclaration*>(current);

                createGlobal(var->name.data.name);
                if (var->value != nullptr) {
                    compileExpression(var->value);

                    emitBytes(Bytecode::SET_GLOBAL, resolveGlobal(var->name.data.name));
                    emitByte(Bytecode::POP);
                }
            }
        }

        declareNatives();

        for (Stmt::Function* func : funcs) {
            Stmt::Function function = *func;
            compileFunction(function);
        }

        if (!errors.empty()) {
            std::string messages;

            for (const std::string& message : errors) {
                messages += message + "\n";
            }

            throw CompileError(messages);
        }

        loadNamedVariable("main");
        emitBytes(Bytecode::CALL, 0);
        emitByte(Bytecode::EXIT);

        program.globalCount = globals.size();
        program.code = init;

        return program;
    }


    ExprVisitResults visitAssignExpr(Expr::Assign *expr) override {

        compileExpression(expr->value);

        i32 index = resolveGlobal(expr->name.data.name);
        Bytecode::Operation op = Bytecode::SET_GLOBAL;

        if (index == -1) {
            index = resolveLocal(expr->name.data.name);
            op = Bytecode::SET_LOCAL;
        }

        emitBytes(op, (u8) index);

        return std::monostate();
    }

    ExprVisitResults visitBinaryExpr(Expr::Binary *expr) override {
        compileExpression(expr->left);
        compileExpression(expr->right);


        switch (expr->operatorType) {
            case Lexing::PLUS: emitByte(Bytecode::ADD); break;
            case Lexing::MINUS: emitByte(Bytecode::SUBTRACT); break;
            case Lexing::STAR: emitByte(Bytecode::MULTIPLY); break;
            case Lexing::SLASH: emitByte(Bytecode::DIVIDE); break;

            case Lexing::EQUALS_EQUALS: emitByte(Bytecode::EQUALS); break;
            case Lexing::BANG_EQUALS: emitBytes(Bytecode::EQUALS, Bytecode::NOT); break;

            case Lexing::LESS: emitByte(Bytecode::LESS); break;
            case Lexing::LESS_EQUALS: emitByte(Bytecode::LESS_EQUALS); break;
            case Lexing::MORE: emitByte(Bytecode::MORE); break;
            case Lexing::MORE_EQUALS: emitByte(Bytecode::MORE_EQUALS); break;

            default:
                throw CompileError("Invalid binary token " + Lexing::Token::toString(expr->operatorType));
        }

        return std::monostate();
    }

    ExprVisitResults visitCallExpr(Expr::Call *expr) override {
        compileExpression(expr->callee);

        for (Expr::Expr* arg : expr->args) {
            compileExpression(arg);
        }

        u8 argCount = (u8) expr->args.size();
        emitBytes(Bytecode::CALL, argCount);

        return std::monostate();
    }

    ExprVisitResults visitConditionalExpr(Expr::Conditional *expr) override {
        compileExpression(expr->condition);
        u64 elseJump = emitJump(Bytecode::JUMP_FALSE);

        emitByte(Bytecode::POP); // pop condition (truthy path)

        compileExpression(expr->thenBranch);

        u64 exitJump = emitJump(Bytecode::JUMP);

        patchJump(elseJump); // begin falsy path

        emitByte(Bytecode::POP); // pop condition

        compileExpression(expr->elseBranch);

        patchJump(exitJump);

        return std::monostate();
    }

    ExprVisitResults visitIdentifierExpr(Expr::Identifier *expr) override {
        loadNamedVariable(expr->target.data.name);
        return std::monostate();
    }

    ExprVisitResults visitLogicalExpr(Expr::Logical *expr) override {
        Bytecode::Operation jumpType;
        Bytecode::Operation first, second;

        if (expr->operatorType == Lexing::AMP_AMP) {
            jumpType = Bytecode::JUMP_FALSE;
            first = Bytecode::TRUE;
            second = Bytecode::FALSE;
        } else if (expr->operatorType == Lexing::PIPE_PIPE) {
            jumpType = Bytecode::JUMP_TRUE;
            first = Bytecode::FALSE;
            second = Bytecode::TRUE;
        } else throw CompileError("Invalid logical expression operator: " + Lexing::Token::toString(expr->operatorType));

        compileExpression(expr->left);
        u64 aJump = emitJump(jumpType);

        emitByte(Bytecode::POP);
        compileExpression(expr->right);
        u64 bJump = emitJump(jumpType);

        emitBytes(Bytecode::POP, first);
        u64 firstJump = emitJump(Bytecode::JUMP);

        patchJump(aJump);
        patchJump(bJump);

        emitBytes(Bytecode::POP, second);

        patchJump(firstJump);

        return std::monostate();
    }

    ExprVisitResults visitNumberExpr(Expr::Number *expr) override {

        if (expr->value == 0) {
            emitByte(Bytecode::FALSE);
            return std::monostate();
        }

        if (expr->value == 1) {
            emitByte(Bytecode::TRUE);
            return std::monostate();
        }

        double fraction;
        if (std::modf(expr->value, &fraction) == 0 && INT8_MIN <= expr->value && INT8_MAX >= expr->value) {
            // use LOAD_I8

            i8 value = static_cast<i8>(expr->value);

            u8 number = *((u8*) &value);

            emitBytes(Bytecode::LOAD_I8, number);

            return std::monostate();
        }

        // use LOAD_CONSTANT

        emitConstant(expr->value);

        return std::monostate();
    }

    ExprVisitResults visitUnaryExpr(Expr::Unary *expr) override {
        compileExpression(expr->right);

        Bytecode::Operation op;
        switch (expr->operatorType) {
            case Lexing::MINUS: op = Bytecode::NEGATE; break;
            case Lexing::BANG: op = Bytecode::NOT; break;

            default: throw CompileError("Unknown unary type"); // unreachable
        }

        emitByte(op);

        return std::monostate();
    }



    StmtVisitResults visitBlockStmt(Stmt::Block *stmt) override {
        beginScope();

        for (Stmt::Stmt* current : stmt->statements) {
            compileStmt(current);
        }

        u8 popCount = countCurrentScopeVars();

        if (popCount > 0) {
            if (popCount == 1) emitByte(Bytecode::POP);
            else emitBytes(Bytecode::POP_N, popCount);
        }

        endScope();

        return std::monostate();
    }

    StmtVisitResults visitExpressionStmt(Stmt::Expression *stmt) override {
        compileExpression(stmt->expression);
        emitByte(Bytecode::POP);
        return std::monostate();
    }

    StmtVisitResults visitFunctionStmt(Stmt::Function *stmt) override {
        Stmt::Function function = *stmt;
        compileFunction(function);

        return std::monostate();
    }

    StmtVisitResults visitIfStmt(Stmt::If *stmt) override {
        compileExpression(stmt->condition);

        u64 elseJump = emitJump(Bytecode::JUMP_FALSE);
        emitByte(Bytecode::POP); // pop condition (truthy branch)

        compileStmt(stmt->thenBranch);

        u64 thenJump = emitJump(Bytecode::JUMP);

        patchJump(elseJump);

        emitByte(Bytecode::POP); // pop condition (falsy path)
        if (stmt->elseBranch != nullptr) compileStmt(stmt->elseBranch);

        patchJump(thenJump);

        return std::monostate();
    }

    StmtVisitResults visitPrintStmt(Stmt::Print *stmt) override {
        compileExpression(stmt->expression);
        emitByte(Bytecode::PRINT);

        return std::monostate();
    }

    StmtVisitResults visitReturnStmt(Stmt::Return *stmt) override {
        if (stmt->value != nullptr) compileExpression(stmt->value);
        else emitByte(Bytecode::FALSE);

        emitByte(Bytecode::RETURN);

        return std::monostate();
    }

    StmtVisitResults visitVariableDeclarationStmt(Stmt::VariableDeclaration *stmt) override {
        if (stmt->value != nullptr) compileExpression(stmt->value);
        else emitByte(Bytecode::FALSE);

        createLocal(stmt->name.data.name);

        return std::monostate();
    }

    StmtVisitResults visitWhileStmt(Stmt::While *stmt) override {
        u64 loopStart = currentChunk().size();

        compileExpression(stmt->condition);

        u64 exitJump = emitJump(Bytecode::JUMP_FALSE);

        emitByte(Bytecode::POP); // pop the condition true

        compileStmt(stmt->body);

        emitLoop(loopStart);

        patchJump(exitJump);
        emitByte(Bytecode::POP); // pop the condition if exited loop

        return std::monostate();
    }

};

Bytecode::Program BytecodeCompilation::compile(const Compiler::CompileResult& ast) {
    BytecodeCompiler compiler;
    auto program = compiler.compile(ast);

    return program;
}

#endif
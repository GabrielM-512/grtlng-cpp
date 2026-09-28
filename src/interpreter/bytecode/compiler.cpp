#include "compiler.h"

#include <cmath>
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

    i32 scopeDepth = 0;

    std::vector<u8>& currentChunk() {
        return program.code;
    }

    void emitByte(u8 byte) {
        currentChunk().push_back(byte);
    }

    void emitBytes(u8 byte1, u8 byte2) {
        emitByte(byte1);
        emitByte(byte2);
    }

    void emitConstant(f64 value) {
        u32 index = addConstant(VALUE_NUM(value));

        if (index > UINT8_MAX) throw CompileError("Too many constants in a chunk");

        emitBytes(Bytecode::LOAD_CONSTANT, (u8) index);
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
        i32 index = resolveGlobal(name);
        Bytecode::Operation op = Bytecode::LOAD_GLOBAL;

        if (index == -1) {
            index = resolveLocal(name);
            op = Bytecode::LOAD_LOCAL;
        }

        if (index == -1) throw CompileError("Unknown Variable " + name);

        emitBytes(op, (u8) index);
    }


    void compileFunction(const Stmt::Function& function) {

        for (Stmt::Stmt* stmt : function.body->statements) {
            compileStmt(stmt);
        }
    }

    void compileExpression(Expr::Expr* expr) {
        expr->accept(this);
    }

    void compileStmt(Stmt::Stmt* stmt) {
        stmt->accept(this);
    }

public:
    Bytecode::Program compile(const Compiler::CompileResult& ast) {
        std::vector<Stmt::Function*> funcs;
        for (Stmt::Stmt* current : ast.tree) {
            if (auto func = dynamic_cast<Stmt::Function*> (current)) {
                createGlobal(func->name.data.name);

                if (!func->body->statements.empty() && !dynamic_cast<Stmt::Return*>(func->body->statements.back())) {

                    static Expr::Number zeroExpr(0.0);
                    static Stmt::Return zeroReturn(&zeroExpr);

                    func->body->statements.push_back(&zeroReturn);
                }

                funcs.push_back(func);
            } else {
                auto var = dynamic_cast<Stmt::VariableDeclaration*>(current);

                createGlobal(var->name.data.name);
                if (var->value != nullptr) compileExpression(var->value);

                emitBytes(Bytecode::SET_GLOBAL, resolveGlobal(var->name.data.name));
                emitByte(Bytecode::POP);
            }
        }

        for (Stmt::Function* func : funcs) {
            Stmt::Function function = *func;
            compileFunction(function);
        }

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

    ExprVisitResults visitCallExpr(Expr::Call *) override {
        throw CompileError("Unimplemented Expression Type: Call");
    }

    ExprVisitResults visitIdentifierExpr(Expr::Identifier *expr) override {
        loadNamedVariable(expr->target.data.name);
        return std::monostate();
    }

    ExprVisitResults visitLogicalExpr(Expr::Logical *) override {
        throw CompileError("Unimplemented Expression Type: Logical");
    }

    ExprVisitResults visitNumberExpr(Expr::Number *expr) override {
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

    StmtVisitResults visitIfStmt(Stmt::If *) override {
        throw CompileError("Unimplemented Statement type: If");
    }

    StmtVisitResults visitPrintStmt(Stmt::Print *stmt) override {
        compileExpression(stmt->expression);
        emitByte(Bytecode::PRINT);

        return std::monostate();
    }

    StmtVisitResults visitReturnStmt(Stmt::Return *stmt) override {
        compileExpression(stmt->value);
        emitByte(Bytecode::RETURN);

        return std::monostate();
    }

    StmtVisitResults visitVariableDeclarationStmt(Stmt::VariableDeclaration *stmt) override {
        if (stmt->value != nullptr) compileExpression(stmt->value);
        else emitByte(Bytecode::FALSE);

        createLocal(stmt->name.data.name);

        return std::monostate();
    }

    StmtVisitResults visitWhileStmt(Stmt::While *) override {
        throw CompileError("Unimplemented Statement type: While");
    }

};

Bytecode::Program BytecodeCompilation::compile(const Compiler::CompileResult& ast) {
    BytecodeCompiler compiler;
    auto program = compiler.compile(ast);

    return program;
}
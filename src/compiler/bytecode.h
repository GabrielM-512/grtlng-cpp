#pragma once
#include <unordered_map>

#include "../interpreter/interpreter.h"
#include "../AST/expr.h"
#include "../AST/stmt.h"

#include "compiler.h"


namespace Bytecode {

    struct Local {
        std::string name;
        i32 depth;
    };

    enum BreakType : u32 {
        BREAK_LOOP,
        BREAK_SWITCH
    };

    // loops and switch statements
    struct BreakStop {
        BreakType type;
        i32 localsSize {};
        std::vector<u64> breaks {};
        std::vector<u64> continues {};
    };

    class CompileError : std::runtime_error {
     public:
          std::string message;

          explicit CompileError(const std::string& message) : runtime_error(message), message(message) {}
     };

     class BytecodeCompiler : public Stmt::StmtVisitor, public Expr::ExprVisitor{
        Program program {};
        std::unordered_map<std::string, u16> globals;
        std::vector<Local> locals;
        std::vector<BreakStop> breakables;

        std::vector<u8> currentFunction;
        std::vector<u8> init;

        bool inInit = true;
        std::vector<std::string> errors;

        i32 scopeDepth = 0;

        std::vector<u8>& currentChunk();

        void emitByte(u8 byte);
        void emitBytes(u8 byte1, u8 byte2);

        void emitConstant(Value::Value value);
        void emitConstant(f64 value);
        u32 addConstant(Value::Value value);

        void emitPops(u8 popCount);

        u16 createGlobal(const std::string& name);
        void createLocal(const std::string& name);

        i32 resolveGlobal(const std::string& name);
        [[nodiscard]] i16 resolveLocal(const std::string& name) const;

        void beginScope();
        void endScope();

        [[nodiscard]] u32 countCurrentScopeVars() const;
        void loadNamedVariable(const std::string& name);

        u64 emitJump(Operation operation);
        void patchJump(u64 jumpLocation);
        void emitLoop(u64 loopStart);

        void compileFunction(const Stmt::Function* function);

        void compileExpression(Expr::Expr* expr);
        void compileStatement(Stmt::Stmt* stmt);

        void declareNatives();

        void startBreakable(BreakType type);
        void startLoop();
        void startSwitch();
        void endLoop();
        BreakStop& findLoop();

        BreakStop& findSwitch();
        void compileSwitchCase(const Stmt::Case& case_);

    public:
        Program compile(const Compiler::SyntaxTree& ast);

        ExprVisitResults visitAssignExpr(Expr::Assign *expr) override;
        ExprVisitResults visitBinaryExpr(Expr::Binary *expr) override;
        ExprVisitResults visitCallExpr(Expr::Call *expr) override;
        ExprVisitResults visitConditionalExpr(Expr::Conditional *expr) override;
        ExprVisitResults visitIdentifierExpr(Expr::Identifier *expr) override;
        ExprVisitResults visitLogicalExpr(Expr::Logical *expr) override;
        ExprVisitResults visitNumberExpr(Expr::Number *expr) override;
        ExprVisitResults visitUnaryExpr(Expr::Unary *expr) override;


        StmtVisitResults visitBlockStmt(Stmt::Block *stmt) override;
        StmtVisitResults visitBreakStmt(Stmt::Break *) override;
        StmtVisitResults visitContinueStmt(Stmt::Continue *) override;
        StmtVisitResults visitExpressionStmt(Stmt::Expression *stmt) override;
        StmtVisitResults visitForStmt(Stmt::For *stmt) override;
        StmtVisitResults visitFunctionStmt(Stmt::Function *stmt) override;
        StmtVisitResults visitIfStmt(Stmt::If *stmt) override;
        StmtVisitResults visitPrintStmt(Stmt::Print *stmt) override;
        StmtVisitResults visitReturnStmt(Stmt::Return *stmt) override;
        StmtVisitResults visitVariableDeclarationStmt(Stmt::VariableDeclaration *stmt) override;
        StmtVisitResults visitWhileStmt(Stmt::While *stmt) override;
        StmtVisitResults visitSwitchStmt(Stmt::Switch *stmt) override;

    };

    Program compile(const Compiler::SyntaxTree& tree);
}

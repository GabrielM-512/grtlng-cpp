#pragma once
#include <unordered_map>

#include "compiler.h"

namespace Resolving {
    class Scope {
        std::unordered_map<std::string, bool> symbols;
    public:
        Scope* enclosing;

        Scope(const Scope&) = delete;

        Scope(): enclosing(nullptr) {}

        Scope(Scope* enclosing, const Scope& base): enclosing(enclosing) {
            for (auto current : base.symbols) {
                symbols.insert(current);
            }
        }

        Scope(Scope* enclosing): enclosing(enclosing) {}
        ~Scope() = default;

        bool hasVar(const std::string& name) const {
            return symbols.contains(name);
        }

        void createVar(const std::string& name) {
            symbols.insert({name, false});
        }

        void activateVar(const std::string& name) {
            symbols.find(name)->second = true;
        }

        bool isActivated(const std::string& name) const {
            return symbols.find(name)->second;
        }
    };
    void resolve(Compiler::CompileResult &program, Error::ErrorHandler &handler);
}

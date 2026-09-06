"""
Classes:
    [ClassName] | [type + name] = [default], [type + name] = [default], ... [type + name] = [default]

"""

def define_subclass(base_class : str, new_class : str) -> str:
    name = new_class.split("|")[0].strip()
    args = new_class.split("|")[1].split(",")

    for i in range(len(args)):
        args[i] = args[i].strip()

    output = f"\n\n    struct {name}: {base_class} " + "{\n"

    for arg in args:
        output += f"        {arg.split("=")[0].strip()};\n"

    output += f"\n        explicit {name}(" # constructor

    for arg in args:
        output += f"\n            {arg},"

    output = output[:-1] + "\n        ):" # remove trailing comma

    for arg in args:
        identifier = arg.split(" = ")[0].strip().split(" ")[-1]
        output += f" {identifier}({identifier}),"

    output = output [:-1] + " {}\n\n" # remove trailing comma

    output += (f"        void accept({base_class}Visitor* visitor) override " + "{\n" +
               f"            visitor->visit{name}{base_class}(this);\n" +
               "        }\n")


    output += "    };" # close class

    return output


def define_ast(output_dir : str, base_class : str, classes : list[str], includes : str = "") -> None:
    output = "#pragma once\n\n"

    for include in includes.split("|"):
        output += f"#include {include}\n"

    output += f"\nnamespace {base_class} " + '{\n'

    for new_class in classes:
        name = new_class.split("|")[0].strip()
        output += "    struct " + name + ";\n" # forward declare classes

    output += (f"\n    class {base_class}Visitor " + "{\n" +
                "    public:\n" +
               f"        virtual ~{base_class}Visitor() = default;\n\n")

    for new_class in classes:
        name = new_class.split("|")[0].strip()
        output += f"        virtual void visit{name}{base_class}(const {name}* {base_class.lower()}) const = 0;\n"

    output += ("    };\n\n" +
              f"    struct {base_class} " + "{\n" +
              f"        virtual ~{base_class}() = default;\n" +
              f"        virtual void accept({base_class}Visitor *visitor) = 0;\n" +
               "    };")

    for new_class in classes:
        output += define_subclass(base_class, new_class)


    output += "\n}" # close namespace

    with open(output_dir, "w") as f:
        f.write(output)


if __name__ == "__main__":
    define_ast("/home/gabriel/CLionProjects/grtlng-cpp/src/AST/expr.h", "Expr",
               [
                  "Binary | const Expr* left = nullptr, Lexing::Tokens::TokenType operatorType = Lexing::Tokens::ERROR, const Expr* right = nullptr",
                  "Unary | Lexing::Tokens::TokenType operatorType = Lexing::Tokens::ERROR, const Expr* right = nullptr",
                  "Number | double value = 0",
                  "Identifier | const char* target"
              ],
               "\"../compiler/lexing.h\"")
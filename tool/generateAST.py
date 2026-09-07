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

    output += (f"        {base_class}VisitResults accept({base_class}Visitor* visitor) override " + "{\n" +
               f"            return visitor->visit{name}{base_class}(this);\n" +
               "        }\n")


    output += "    };" # close class

    return output


def define_visit_results(base_class : str, visit_results : str) -> str:
    output = f"    #define {base_class}VisitResults std::variant<"

    for result in visit_results.split("|"):
        name = result.strip()
        output += name + ", "

    output = output[:-2] + ">"

    return output


def define_ast(output_dir : str, base_class : str, classes : list[str], visit_results : str, includes : str = "") -> None:
    output = "#pragma once\n\n#include <variant>\n\n"

    for include in includes.split("|"):
        output += f"#include {include}\n"

    output += f"\nnamespace {base_class} " + '{\n'

    for new_class in classes:
        name = new_class.split("|")[0].strip()
        output += "    struct " + name + ";\n" # forward declare classes

    output += "\n" + define_visit_results(base_class, visit_results) + "\n"

    output += (f"\n    class {base_class}Visitor " + "{\n" +
                "    public:\n" +
               f"        virtual ~{base_class}Visitor() = default;\n\n")

    for new_class in classes:
        name = new_class.split("|")[0].strip()
        output += f"        virtual {base_class}VisitResults visit{name}{base_class}({name}* {base_class.lower()}) = 0;\n"

    output += ("    };\n\n" +
              f"    struct {base_class} " + "{\n" +
              f"        virtual ~{base_class}() = default;\n" +
              f"        virtual {base_class}VisitResults accept({base_class}Visitor *visitor) = 0;\n" +
               "    };")

    for new_class in classes:
        output += define_subclass(base_class, new_class)


    output += "\n}" # close namespace

    with open(output_dir, "w") as f:
        f.write(output)


if __name__ == "__main__":
    define_ast(output_dir="/home/gabriel/CLionProjects/grtlng-cpp/src/AST/expr.h",
               base_class="Expr",
               classes = [
                  "Binary | Expr* left = nullptr, Lexing::Tokens::TokenType operatorType = Lexing::Tokens::ERROR, Expr* right = nullptr",
                  "Unary | Lexing::Tokens::TokenType operatorType = Lexing::Tokens::ERROR, Expr* right = nullptr",
                  "Number | double value = 0",
                  "Identifier | const char* target"
              ],
               visit_results= "std::string, Value::Value",
               includes = "\"../compiler/lexing.h\" | \"../value.h\"")
#include "argvParse.h"
#include <argp.h>
#include <cstdlib>
#include <cstring>
#include <sysexits.h>

using namespace argvParse;

error_t parse_opt (int key, char *arg, argp_state *state) {
    auto arguments = static_cast<ProgramArgs *>(state->input);

    switch (key) {
        case 'c': {
            if (strcmp(arg, "i") == 0) {
                if (arguments->type != NONE) {
                    argp_error(state, "Set compile mode more than once");
                } else {
                    arguments->type = INTERPRET;
                }
            } else {
                argp_failure(state, EX_USAGE, EINVAL, "Bad option \"%s\" to -c\nExpected options: i", arg);
            }
            break;
        }

        case 'd': {
            arguments->decompile = true;
            break;
        }

        case 256: {
            arguments->printAst = true;
            break;
        }

        case ARGP_KEY_ARG: {
            if (arguments->filePath == nullptr) {
                arguments->filePath = (char*) malloc(strlen(arg));
                strcpy(arguments->filePath, arg);
            } else {
                argp_failure(state, EX_USAGE, EINVAL, "Supplied more than one file path to executable");
            }
            break;
        }

        case ARGP_KEY_END: {
            if (arguments->filePath == nullptr) {
                argp_failure(state, EX_USAGE, 0, "Expected a filepath");
            }
            break;
        }

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

ProgramArgs argvParse::parse(int argc, char* argv[]) {

    static char doc[] = "A compiler and interpreter for the grtlng programming language";
    static char args_doc[] = "./grtlng [source file] [mode] [options]";

    static argp_option options[] = {
        {.name = "compile-mode", .key = 'c', .arg = "type", .flags = 0, .doc = "[-c<value>] The mode of the compiler. Possible options: i (interpret program)", .group = 0},
        {.name = nullptr, .key = 'd', .arg = nullptr, .flags = 0, .doc = "[-d] Whether to decompile the bytecode before execution. Only valid when used in combination with -ci.", .group = 0},
        {.name = "print-ast", .key = 256, .arg = nullptr, .flags = 0, .doc = "[--print-ast] Prints the AST for the program using parentheses in expressions to explicitly show precedence.", .group = 0},
        {.name = nullptr, .key = 0, .arg = nullptr, .flags = 0, .doc = nullptr, .group = 0} // last entry
    };

    argp parser {
        .options = options,
        .parser = parse_opt,
        .args_doc = args_doc,
        .doc = doc,
        .children = nullptr,
        .help_filter = nullptr,
        .argp_domain = nullptr
    };

    ProgramArgs args;

    argp_parse(&parser, argc, argv, 0, nullptr, &args);

    return args;


}
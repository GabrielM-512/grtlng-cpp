#include "argvParse.h"
#include <argp.h>
#include <cstdlib>
#include <cstring>
#include <sysexits.h>

using namespace argvParse;

struct parseInfos {
    ProgramArgs args;
    bool setMode = false;
};

error_t parse_opt (int key, char *arg, argp_state *state) {
    auto arguments = static_cast<parseInfos *>(state->input);

    switch (key) {
        case 'c': {
            if (arguments->setMode) {
                argp_error(state, "Set compile mode more than once");
            }

            arguments->setMode = true;

            if (strcmp(arg, "i") == 0) arguments->args.type = INTERPRET;
            else if (strcmp(arg, "n") == 0) arguments->args.type = NONE;

            else {
                argp_error(state, "Bad option \"%s\" to -c\n    Valid options: i, n", arg);
            }

            break;
        }

        case 'd': {
            arguments->args.decompile = true;
            break;
        }

        case 256: {
            arguments->args.printAst = true;
            break;
        }

        case ARGP_KEY_ARG: {
            if (arguments->args.filePath == nullptr) {
                arguments->args.filePath = (char*) malloc(strlen(arg));
                strcpy(arguments->args.filePath, arg);
            } else {
                argp_error(state, "Supplied more than one file path to executable");
            }
            break;
        }

        case ARGP_KEY_END: {
            if (arguments->args.filePath == nullptr) {
                argp_error(state, "Expected a filepath");
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
        {.name = "compile-mode", .key = 'c', .arg = "type", .flags = 0, .doc = "The mode of the compiler. Possible options: i (interpret program), n (none). Default: i", .group = 0},
        {.name = nullptr, .key = 'd', .arg = nullptr, .flags = 0, .doc = "Whether to decompile the bytecode before execution. Only valid when used in combination with -ci.", .group = 0},
        {.name = "print-ast", .key = 256, .arg = nullptr, .flags = 0, .doc = "Prints the AST for the program using parentheses in expressions to explicitly show precedence.", .group = 0},
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

    parseInfos args;

    argp_parse(&parser, argc, argv, 0, nullptr, &args);

    return args.args;


}
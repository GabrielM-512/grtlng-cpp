#include "programDump.h"

#include "dumping/bytecode.h"

using namespace programDump;

void programDump::dumpBytecode(const Bytecode::Program &program, const char *filepath) {
    Bytecode::dump(program, filepath);
}


Bytecode::Program programDump::readBytecodeFile(const char *filepath) {
    return Bytecode::load(filepath);
}
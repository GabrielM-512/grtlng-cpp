#include "bytecode.h"

#include "../fileIO.h"
#include "../programDump.h"

#define OFFSET_NATIVE_COUNT 0x0C
#define OFFSET_GLOBAL_COUNT 0x10
#define OFFSET_POINTER_TABLE 0x12

/*
    file format:
        Header:
            0E 4E 0B 89 // 0x0000
            u64: checksum, excluding Header section // 0x0004

        Body:
            u32: native func count // 0x000C
            u16: global count // 0x0010

            Data Pointer Table:
                u64: Object Table pos // 0x0012
                u64: Constant Table pos // 0x001A
                u64: init code pos // 0x0022
                u64: binary data start // 0x002A

            Object Table:
                <u64> object binary data offset positions

            Constant Table
                u64 constant count
                <Value> values

            init code:
                u64 size
                <data>
            binary data:
                <data>

        Representations:
            Value Representation:
                u64: type tag (same as ValueType enum)
                union: {
                    f64: Number value
                    u64: object table index
                }

            Object Representation:
                all: u8: object type (same as ObjectType enum)

                native func:
                    none

                function:
                    u8: function arity
                    u64: code size
                    // string after code data, so at code + code size
                    <code>
*/

struct DumpedValue {
    u64 type;
    union {
        f64 number;
        u64 objectIndex;
    };
};

struct ObjectTable {
    std::vector<u64> objects;
};

struct ConstantTable {
    std::vector<DumpedValue> values;
};

struct ProgramDump {
    std::vector<u8> data;
    ObjectTable objects;
    ConstantTable constants;
    std::vector<u8> binaryData;
};

/*
    N   N   U   U   M   M   BBBB    EEEEE   RRRR            DDDD    U   U   M   M   PPPP     SSSS
    NN  N   U   U   MM MM   B   B   E       R   R           D   D   U   U   MM MM   P   P   S
    N N N   U   U   M M M   BBBB    EEEEE   RRRR            D   D   U   U   M M M   PPPP     SSS
    N  NN   U   U   M   M   B   B   E       R  R            D   D   U   U   M   M   P           S
    N   N    UUU    M   M   BBBB    EEEEE   R   R           DDDD     UUU    M   M   P       SSSS
*/

static void dumpu8(std::vector<u8> &out, u8 data) {
    out.push_back(data);
}

static void dumpData(std::vector<u8> &out, const u8* data, u64 size) {
    for (u64 i = 0; i < size; i++) {
        dumpu8(out, data[i]);
    }
}

static void dumpu16(std::vector<u8> &out, u16 data) {
    dumpData(out, (u8*) &data, sizeof(data));
}

static void dumpu32(std::vector<u8> &out, u32 data) {
    dumpData(out, (u8*) &data, sizeof(data));
}

static void dumpu64(std::vector<u8> &out, u64 data) {
    dumpData(out, (u8*) &data, sizeof(data));
}

static void dumpf64(std::vector<u8> &out, f64 data) {
    dumpData(out, (u8*) &data, sizeof(data));
}

static void dumpBinary(std::vector<u8> &out, const std::vector<u8> &data) {
    out.insert(std::end(out), std::begin(data), std::end(data));
}

static void reserveu64(ProgramDump &data) {
    u64 reserved = 0xFFFFFFFFFFFFFFFF;

    dumpu64(data.data, reserved);
}

/*
    V   V     A     L       U   U   EEEEE           DDDD    U   U   M   M   PPPP     SSSS
    V   V    A A    L       U   U   E               D   D   U   U   MM MM   P   P   S
     V V     AAA    L       U   U   EEEEE           D   D   U   U   M M M   PPPP     SSS
     V V    A   A   L       U   U   E               D   D   U   U   M   M   P           S
      V     A   A   LLLLL    UUU    EEEEE           DDDD     UUU    M   M   P       SSSS
*/

static void dumpString(std::vector<u8> &out, const std::string& data) {
    const char* string = data.c_str();
    const u64 length = data.size() * sizeof(string[0]) + 1; // + 1 for null termination
    dumpData(out, reinterpret_cast<const u8*>(string), length);
}

static u16 dumpObj(ProgramDump& dump, Value::Obj* obj) {
    u64 binaryPos = dump.binaryData.size();

    dumpu8(dump.binaryData, obj->type);

    switch (obj->type) {
        case Value::FUNCTION: {
            auto func = static_cast<Value::Function*>(obj);
            dumpu8(dump.binaryData, func->arity);
            dumpu64(dump.binaryData, func->code.size());
            dumpBinary(dump.binaryData, func->code);
            dumpString(dump.binaryData, func->name);
            break;
        }
        case Value::NATIVE_FN:
            break; // nothing to do
    }

    dump.objects.objects.push_back(binaryPos);

    return dump.objects.objects.size() - 1;
}

static void dumpConsts(ProgramDump& dump, const Bytecode::Program& program) {

    for (const Value::Value value : program.constants) {
        DumpedValue dumpVal{};
        dumpVal.type = value.type;

        switch (value.type) {
            case Value::NUMBER: {
                dumpVal.number = AS_NUM(value);
                break;
            }

            case Value::OBJECT: {
                dumpVal.objectIndex = dumpObj(dump, AS_OBJ(value));
                break;
            }
        }

        dump.constants.values.push_back(dumpVal);

    }
}

/*
    U   U   TTTTT   IIIII   L        SSSS
    U   U     T       I     L       S
    U   U     T       I     L        SSS
    U   U     T       I     L           S
     UUU      T     IIIII   LLLLL   SSSS
*/

static u32 countNatives(const Bytecode::Program& program) {
    u32 count = 0;
    for (Value::Value val : program.constants) {
        if (!IS_NATIVE(val)) break;
        count++;
    }

    return count;
}

static void finishDump(ProgramDump &dump, const Bytecode::Program &program) {
    const u64 objectTablePos = dump.data.size();
    for (const u64 objPos : dump.objects.objects) {
        dumpu64(dump.data, objPos);
    }

    const u64 constantTablePos = dump.data.size();

    dumpu64(dump.data, program.constants.size());

    for (const DumpedValue value : dump.constants.values) {
        dumpu64(dump.data, value.type);
        dumpf64(dump.data, value.number);
    }

    const u64 initCodePos = dump.data.size();
    dumpu64(dump.data, program.code.size());
    dumpBinary(dump.data, program.code);

    const u64 binaryDataPos = dump.data.size();
    dumpBinary(dump.data, dump.binaryData);

    u64* dataTable = (u64*) (dump.data.data() + OFFSET_POINTER_TABLE); // skip header, native count, global count

    dataTable[0] = objectTablePos;
    dataTable[1] = constantTablePos;
    dataTable[2] = initCodePos;
    dataTable[3] = binaryDataPos;

    // TODO: checksum
}



void Bytecode::dump(const Program& program, const char* filepath) {
    ProgramDump dump;
    dump.data = {
        // magic number
        0x0E, 0x4E, 0x0B, 0x59
    };

    // reserve checksum
    reserveu64(dump);

    u32 nativeCount = countNatives(program);
    dumpu32(dump.data, nativeCount);

    dumpu16(dump.data, program.globalCount);

    // reserve Data Pointer Table size
    reserveu64(dump); // Object Table
    reserveu64(dump); // Constant Table
    reserveu64(dump); // init code
    reserveu64(dump); // binary data

    dumpConsts(dump, program);

    finishDump(dump, program);

    fileIO::writeFile(filepath, dump.data);
}


/*
    L        OOO      A     DDDD    IIIII   N   N    GGG
    L       O   O    A A    D   D     I     NN  N   G
    L       O   O    AAA    D   D     I     N N N   G  GG
    L       O   O   A   A   D   D     I     N  NN   G   G
    LLLLL    OOO    A   A   DDDD    IIIII   N   N    GGG
*/

struct LoadData {
    const u8* objects;
    const u8* consts;
    const u8* init;
    const u8* binary;
    Bytecode::Program& program;
};

std::vector<u8> loadCode(u64 codeLen, const u8* codeStart) {
    std::vector<u8> code;
    for (u64 i = 0; i < codeLen; i++) {
        code.push_back(codeStart[i]);
    }

    return code;
}

bool checkHeader(std::vector<u8> file) {
    constexpr u8 magicNum[] = {0x0E, 0x4E, 0x0B, 0x59};

    for (u8 i = 0; i < 4; i++) {
        if (file.at(i) != magicNum[i]) return false;
    }

    return true;
}

const u8* getObjectPos(u64 objectIndex, LoadData& data) {
    const u64* table = (u64*) data.objects;
    u64 binaryOffset = table[objectIndex];

    return data.binary + binaryOffset;
}

std::optional<Value::Obj*> loadObject(u64 objectIndex, LoadData& data) {

    const u8* objData = getObjectPos(objectIndex, data);
    auto type = static_cast<Value::ObjectType>(*objData);

    switch (type) {
        case Value::FUNCTION: {
            const u8 arity = *(objData + 1);

            const u64 codeLen = *(u64*) (objData + 2);
            const u8* codePtr = objData + 10;
            std::vector<u8> code = loadCode(codeLen, codePtr);

            const char* name = (const char*) codePtr + codeLen;


            return new Value::Function(code, name, arity);
        }

        case Value::NATIVE_FN:
            return std::nullopt;
    }

    throw programDump::LoadError("Invalid Object Type");
}

std::optional<Value::Value> loadValue(const u64* valuePos, LoadData& data) {
    auto type = static_cast<Value::ValueType>(*valuePos);

    switch (type) {
        case Value::NUMBER:
            return VALUE_NUM(*(f64*) (valuePos + 1));
        case Value::OBJECT: {
            auto obj = loadObject(*(valuePos + 1), data);
            if (!obj) return std::nullopt;

            return VALUE_OBJ(obj.value());
        }
    }

    throw programDump::LoadError("Invalid Value Type");

}

void loadProgram(LoadData& data) {
    const u64 constCount = *((u64*) data.consts);

    for (u64 i = 0; i < constCount; i++) {
        u64* valuePtr = ((u64*) data.consts) + (1 + 2 * i); // 1 to skip constCount, 2 * i because each value has two u64s
        auto val = loadValue(valuePtr, data);

        if (!val) continue;

        if (IS_NATIVE(val.value())) continue;

        data.program.constants.push_back(val.value());
    }

    u64 initLength = *((u64*) data.init);
    const u8* initCode = data.init + sizeof(u64);

    data.program.code = loadCode(initLength, initCode);

}

Bytecode::Program Bytecode::load(const char *filepath) {
    std::vector<u8> file = fileIO::readBinaryFile(filepath);

    if (file.size() < 0x32) throw programDump::LoadError("File was too small to hold binary data");

    if (!checkHeader(file)) throw programDump::LoadError("Magic number was invalid");
    // TODO: check checksum


    // TODO: working with raw pointers is probably unsafe if the file was tampered with, maybe theres some way to work with r/w iterators to ensure we don't overrun the buffer

    u8* base = file.data();

    u64* pointerTable = reinterpret_cast<u64*>(base + OFFSET_POINTER_TABLE);

    u8* objectTable = base + pointerTable[0];
    u8* constantTable = base + pointerTable[1];
    u8* initCode = base + pointerTable[2];
    u8* binaryData = base + pointerTable[3];

    Program program;

    LoadData data = {
        .objects = objectTable,
        .consts = constantTable,
        .init = initCode,
        .binary = binaryData,
        .program = program
    };

    {
        u32 nativeCount = *(u32*) (base + OFFSET_NATIVE_COUNT);
        auto funcs = Value::nativeFnDefinitions();
        for (u32 i = 0; i < nativeCount; i++) {
            auto* func = new Value::NativeFn(funcs.at(i));
            program.constants.push_back(VALUE_NATIVE(func));
        }
    }

    program.globalCount = *(u16*) (base + OFFSET_GLOBAL_COUNT);

    loadProgram(data);

    return program;
}
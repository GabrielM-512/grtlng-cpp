#include "value.h"

#include <cstring>

#include "iostream"
#include "compiler/lexing.h"
#include "interpreter/runtimeException.h"

#include "compiler/resolving.h"

/*
    N   N     A     TTTTT   IIIII   V   V   EEEEE           FFFFF   U   U   N   N     CCC   TTTTT   IIIII    OOO    N   N    SSSS
    NN  N    A A      T       I     V   V   E               F       U   U   NN  N    C        T       I     O   O   NN  N   S
    N N N    AAA      T       I      V V    EEEEE           FFFFF   U   U   N N N   C         T       I     O   O   N N N    SSS
    N  NN   A   A     T       I      V V    E               F       U   U   N  NN    C        T       I     O   O   N  NN       S
    N   N   A   A     T     IIIII     V     EEEEE           F        UUU    N   N     CCC     T     IIIII    OOO    N   N   SSSS
*/

Value::Value clockNative(Value::Value*) {
    return VALUE_NUM((double) clock() / CLOCKS_PER_SEC);
}

std::vector<Value::NativeFn> Value::nativeFnDefinitions() {
    std::vector fns = {
        NativeFn(clockNative, "clock", 0),
    };

    return fns;
}


void defineNativeResolver(Resolving::Scope* scope, const char* name) {
    scope->createVar(name);
    scope->activateVar(name);
}

void Value::defineNativesResolver(Resolving::Scope *scope) {
    defineNativeResolver(scope, "clock");
}

/*
     OOO    BBBB        J   EEEEE     CCC   TTTTT           H   H     A     N   N   DDDD    L       IIIII   N   N    GGG
    O   O   B   B       J   E        C        T             H   H    A A    NN  N   D   D   L         I     NN  N   G
    O   O   BBBB        J   EEEEE   C         T             HHHHH    AAA    N N N   D   D   L         I     N N N   G  GG
    O   O   B   B   J   J   E        C        T             H   H   A   A   N  NN   D   D   L         I     N  NN   G   G
     OOO    BBBB     JJJ    EEEEE     CCC     T             H   H   A   A   N   N   DDDD    LLLLL   IIIII   N   N    GGG
*/

bool Value::isObjType(const Value& val, ObjectType type) {
    return val.type == OBJECT && val.as.object->type == type;
}

bool objsEqual(const Value::Obj* a, const Value::Obj* b) {
    if (a->type != b->type) return false;

    switch (a->type) {
        case Value::NATIVE_FN: {
            const auto funcA = static_cast<const Value::NativeFn*>(a);
            const auto funcB = static_cast<const Value::NativeFn*>(b);

            return funcA->fn == funcB->fn;
        }
        case Value::FUNCTION: {
            return a == b;
        }
    }

    return false;
}

void Value::printValue(const Value& value) {
    std::cout << getValueString(value) << std::endl;
}

std::string getObjectString(const Value::Obj* obj) {
    switch (obj->type) {
        case Value::FUNCTION: {
            auto func = static_cast<const Value::Function*>(obj);
            return "<fn \"" + func->name + "\">";
        }

        case Value::NATIVE_FN: {
            auto func = static_cast<const Value::NativeFn*>(obj);
            return "<fn \"" + func->name + "\">";
            break;
        }

        default:
            return "UNKNOWN OBJECT TYPE";
    }

    return "";

}

std::string Value::getValueString(const Value& value) {
    switch (value.type) {
        case NUMBER:
            return std::to_string(AS_NUM(value));
        case OBJECT:
            return getObjectString(value.as.object);
        default:
            return "UNKNOWN VALUE TYPE";
    }
}

bool Value::equality(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case NUMBER: return AS_NUM(a) == AS_NUM(b);
        case OBJECT: return objsEqual(a.as.object, b.as.object);
    }

    return false;
}


bool Value::isTruthy(const Value& value) {
    switch (value.type) {
        case NUMBER:
            return AS_NUM(value) != 0;
        case OBJECT:
            return true;
    }
    throw Interpreting::RuntimeException("Invalid value type", (Lexing::Token) {.type = Lexing::ERROR, .line = 0, .position = 0, .data = {.number = 0}});
}

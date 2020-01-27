#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "natives.h"
#include "object.h"
#include "util.h"
#include "vm.h"

static Value
clockNative(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value
timeNative(int argCount, Value* args) {
    return NUMBER_VAL((double)time(NULL));
}

static Value
strNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("str() takes exactly 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (!IS_STRING(args[0])) {
        char* valueString = valueToString(args[0]);

        ObjString* string = copyString(valueString, strlen(valueString));
        free(valueString);

        return OBJ_VAL(string);
    }

    return args[0];
}

static Value
boolNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("bool() takes exactly 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    return BOOL_VAL(!isFalsey(args[0]));
}

static Value
lenNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("bool() takes exactly 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (IS_STRING(args[0]))
        return NUMBER_VAL(AS_STRING(args[0])->length);

    runtimeError("Unsupported type passed to len()");
    return NIL_VAL;
}

const char* nativeNames[] = {
    "clock", "time", "str", "bool", "len",
};

NativeFn nativeFunctions[] = {
    clockNative, timeNative, strNative, boolNative, lenNative,
};

static bool
printNative(int argCount, Value* args) {
    if (argCount == 0) {
        printf("\n");
        return true;
    }

    for (int i = 0; i < argCount; ++i) {
        Value value = args[i];
        printValue(value);
        printf("\n");
    }

    return true;
}

static bool
exitNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("input() takes exactly 1 argument (%d given).", argCount);
        return true;
    }

    Value exitCode = args[0];
    if (!IS_NUMBER(exitCode)) {
        runtimeError("exit() takes only a number value.");
        return true;
    }

    exit((int)AS_NUMBER(exitCode));
}

const char* nativeVoidNames[] = {
    "print",
    "exit",
};

NativeFnVoid nativeVoidFunctions[] = {
    printNative,
    exitNative,
};

void
defineAllNatives() {
    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i)
        defineNative(nativeNames[i], nativeFunctions[i]);

    for (uint8_t i = 0;
         i < sizeof(nativeVoidNames) / sizeof(nativeVoidNames[0]);
         ++i)
        defineNativeVoid(nativeVoidNames[i], nativeVoidFunctions[i]);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "natives.h"
#include "memory.h"
#include "vm.h"

static void defineNative(const char *name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int) strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}


static void defineNativeVoid(const char *name, NativeFnVoid function) {
    push(OBJ_VAL(copyString(name, (int) strlen(name))));
    push(OBJ_VAL(newNativeVoid(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

// Native functions
static Value timeNative(int argCount, Value *args) {
    return NUMBER_VAL((double) time(NULL));
}

static Value clockNative(int argCount, Value *args) {
    return NUMBER_VAL((double) clock() / CLOCKS_PER_SEC);
}

static Value inputNative(int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError("input() takes exactly 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (argCount != 0) {
        Value prompt = args[0];
        if (!IS_STRING(prompt)) {
            runtimeError("input() only takes a string argument");
            return NIL_VAL;
        }

        printf("%s", AS_CSTRING(prompt));
    }

    uint8_t len_max = 128;
    uint8_t current_size = len_max;

    char *line = malloc(len_max);

    if (line == NULL) {
        runtimeError("Memory error on input()!");
        return NIL_VAL;
    }

    int c = EOF;
    uint8_t i = 0;
    while ((c = getchar()) != '\n' && c != EOF) {
        line[i++] = (char) c;

        if (i == current_size) {
            current_size = i + len_max;
            line = realloc(line, current_size);
        }
    }

    line[i] = '\0';

    Value l = OBJ_VAL(copyString(line, strlen(line)));
    free(line);

    return l;
}

static bool printNative(int argCount, Value *args) {
    for (int i = 0; i < argCount; ++i) {
        Value value = args[i];
        printValue(value);
        printf("\n");
    }

    return true;
}

void defineAllNatives() {
    char* nativeNames[] = {
        "clock",
        "time",
        "input",
    };

    NativeFn nativeFunctions[] = {
        clockNative,
        timeNative,
        inputNative,
    };

    char* nativeVoidNames[] = {
        "print",
    };

    NativeFnVoid nativeVoidFunctions[] = {
        printNative,
    };


    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i) defineNative(nativeNames[i], nativeFunctions[i]);
    for (uint8_t i = 0; i < sizeof(nativeVoidNames) / sizeof(nativeVoidNames[0]); ++i) defineNativeVoid(nativeVoidNames[i], nativeVoidFunctions[i]);
}

#include <time.h>
#include <stdio.h>

#include "natives.h"
#include "object.h"
#include "vm.h"

static Value
clockNative(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

const char* nativeNames[] = {
  "clock",
};

NativeFn nativeFunctions[] = {
  clockNative,
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

const char* nativeVoidNames[] = {
  "print",
};

NativeFnVoid nativeVoidFunctions[] = {
  printNative,
};

void defineAllNatives() {
    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i) {
        defineNative(nativeNames[i], nativeFunctions[i]);
    }

    for (uint8_t i = 0; i < sizeof(nativeVoidNames) / sizeof(nativeVoidNames[0]); ++i) {
        defineNativeVoid(nativeVoidNames[i], nativeVoidFunctions[i]);
    }
}

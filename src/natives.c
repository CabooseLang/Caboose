#include "natives.h"
#include "vm.h"
#include "time.h"

static void defineNative(const char* name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int) strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

// static void defineNativeVoid(const char* name, NativeFn function) {
//     push(OBJ_VAL(copyString(name, (int) strlen(name))));
//     push(OBJ_VAL(newNativeVoid(function)));
//     tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
//     pop();
//     pop();
// }

static Value timeNative(int argCount, Value* args) {
    return NUMBER_VAL((double) time(NULL));
}

static Value clockNative(int argCount, Value* args) {
	return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

void defineAllNatives() {
    char* nativeNames[] = {
        "time",
        "clock",
    };

    NativeFn* nativeFunctions[] = {
        timeNative,
        clockNative,
    };

    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i) defineNative(nativeNames[i], nativeFunctions[i]);
}

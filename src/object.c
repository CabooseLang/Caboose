#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType)                                         \
    (type*)allocateObject(sizeof(type), objectType)

static Obj*
allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %ld for %d\n", (void*)object, size, type);
#endif

    return object;
}

ObjClosure*
newClosure(ObjFunction* function) {
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++)
        upvalues[i] = NULL;

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;

    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;

    return closure;
}

static ObjString*
allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, NIL_VAL);
    pop();

    return string;
}

static uint32_t
hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++) {
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString*
copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL)
        return interned;

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

static void
printFunction(ObjFunction* function) {
    if (function->name == NULL) {
        printf("<script>");
        return;
    }

    printf("<fn %s>", function->name->chars);
}

void
printObject(Value value) {
    printf("%s", objectToString(value));
}

ObjString*
takeString(char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjFunction*
newFunction() {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);

    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjInstance* newInstance(ObjClass* klass) {
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);
    return instance;
}

ObjNative*
newNative(NativeFn function) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

ObjUpvalue*
newUpvalue(Value* slot) {
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NIL_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

ObjNativeVoid*
newNativeVoid(NativeFnVoid function) {
    ObjNativeVoid* native = ALLOCATE_OBJ(ObjNativeVoid, OBJ_NATIVE_VOID);
    native->function = function;
    return native;
}

ObjClass* newClass(ObjString* name) {
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    return klass;
}

char*
objectToString(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_CLOSURE: {
            ObjClosure* closure = AS_CLOSURE(value);
            char* closureString =
              malloc(sizeof(char) * (closure->function->name->length + 6));
            snprintf(closureString,
                     closure->function->name->length + 6,
                     "<fn %s>",
                     closure->function->name->chars);
            return closureString;
        }

        case OBJ_FUNCTION: {
            ObjFunction* function = AS_FUNCTION(value);
            char* functionString =
              malloc(sizeof(char) * (function->name->length + 6));
            snprintf(functionString,
                     function->name->length + 6,
                     "<fn %s>",
                     function->name->chars);
            return functionString;
        }

        case OBJ_NATIVE_VOID:
        case OBJ_NATIVE: {
            char* nativeString = malloc(sizeof(char) * 12);
            snprintf(nativeString, 12, "%s", "<native fn>");
            return nativeString;
        }

        case OBJ_STRING: {
            ObjString* stringObj = AS_STRING(value);
            char* string = malloc(sizeof(char) * stringObj->length + 3);
            snprintf(string, stringObj->length + 1, "%s", stringObj->chars);
            return string;
        }

        case OBJ_UPVALUE: {
            char* nativeString = malloc(sizeof(char) * 8);
            snprintf(nativeString, 8, "%s", "upvalue");
            return nativeString;
        }

        case OBJ_CLASS: {
            ObjClass* klass = AS_CLASS(value);
            char* classString = malloc(sizeof(char) * (klass->name->length + 9));
            snprintf(classString,klass->name->length + 9,"<class %s>",klass->name->chars);
            return classString;
        }

        case OBJ_INSTANCE: {
            ObjInstance* instance = AS_INSTANCE(value);
            char* instanceString = malloc(sizeof(char) * (instance->klass->name->length + 10));
            snprintf(instanceString, instance->klass->name->length + 10, "%s instance", instance->klass->name->chars);
            return instanceString;
        }
    }

    char* unknown = malloc(sizeof(char) * 8);
    snprintf(unknown, 8, "%s", "unknown");
    return unknown;
}

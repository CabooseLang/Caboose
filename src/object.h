#ifndef caboose_object_h
#define caboose_object_h

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_NATIVE_VOID(value) isObjType(value, OBJ_NATIVE_VOID)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)

#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->function)
#define AS_NATIVE_VOID(value) (((ObjNativeVoid*)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))

typedef enum {
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_NATIVE_VOID,
    OBJ_STRING,
    OBJ_UPVALUE,
    OBJ_CLASS,
    OBJ_INSTANCE,
} ObjType;

struct sObj {
    ObjType type;
    bool isMarked;
    struct sObj* next;
};

typedef struct {
    Obj obj;
    int arity;
    int upvalueCount;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value* args);
typedef bool (*NativeFnVoid)(int argCount, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

typedef struct {
    Obj obj;
    NativeFnVoid function;
} ObjNativeVoid;

struct sObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

typedef struct sUpvalue {
    Obj obj;
    Value* location;
    Value closed;
    struct sUpvalue* next;
} ObjUpvalue;

typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct sObjClass {
    Obj obj;
    ObjString* name;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* klass;
    Table fields;
} ObjInstance;

static inline bool
isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

ObjString*
takeString(char* chars, int length);

ObjString*
copyString(const char* chars, int length);

ObjClosure*
newClosure(ObjFunction* function);

ObjUpvalue*
newUpvalue(Value* slot);

ObjFunction*
newFunction();

ObjNative*
newNative(NativeFn function);

ObjNativeVoid*
newNativeVoid(NativeFnVoid function);

ObjClass*
newClass(ObjString* name);

ObjInstance*
newInstance(ObjClass* klass);

void
printObject(Value value);

char*
objectToString(Value value);

#endif

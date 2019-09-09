#ifndef caboose_vm_h
#define caboose_vm_h

#include "object.h"
#include "table.h"
#include "value.h"
// TODO: Don't depend on frame count for stack count since we have
// stack before frames?
#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
} CallFrame;

typedef struct {
    Value stack[STACK_MAX];
    Value *stackTop;
    int stackCount;
    bool repl;
    const char *scriptName;
    const char *currentScriptName;
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    int currentFrameCount;
    Table globals;
    Table strings;
    ObjString *initString;
    ObjString *replVar;
    ObjUpvalue *openUpvalues;
    size_t bytesAllocated;
    size_t nextGC;
    Obj *objects;
    Obj *listObjects;
    int grayCount;
    int grayCapacity;
    Obj **grayStack;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM(bool repl, const char *scriptName);

void freeVM();

InterpretResult interpret(const char *source);

void push(Value value);

Value peek(int distance);

void runtimeError(const char *format, ...);

Value pop();

bool isFalsey(Value value);

#endif

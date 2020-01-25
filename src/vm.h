#ifndef caboose_vm_h
#define caboose_vm_h

#include "chunk.h"
#include "table.h"
#include "value.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

/**
 * The call frame.
 * @author RailRunner16
 */
typedef struct {
  ObjFunction* function;
  uint8_t* ip;
  Value* slots;
} CallFrame;

/**
 * A Caboose virtual machine.
 * @author RailRunner16
 */
typedef struct {
  Chunk* chunk;
  uint8_t* ip;
  Value stack[STACK_MAX];
  Value* stackTop;
  Obj* objects;
  Table globals;
  Table strings;

  CallFrame frames[FRAMES_MAX];
  int frameCount;
} VM;

/**
 * The result of running the code.
 * @author RailRunner16
 */
typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
} InterpretResult;

/**
 * The main VM instance.
 */
extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();
//void defineNative(const char* name, NativeFn function);

#endif

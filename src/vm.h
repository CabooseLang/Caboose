#ifndef _CABOOSE_VM_H_
#define _CABOOSE_VM_H_

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
	Chunk* chunk;
	uint8_t* ip;
	Value stack[STACK_MAX];
	Value* stackTop;
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void initVM();    
void freeVM();
InterpretResult interpret(Chunk* chunk);

#endif
#ifndef _CABOOSE_VM_H_
#define _CABOOSE_VM_H_

#include "chunk.h"

typedef struct {
	Chunk* chunk;
	uint8_t* ip;
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
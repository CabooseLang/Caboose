#include <stdio.h>

#include "common.h"
#include "vm.h"

VM vm;

void initVM() {
}

void freeVM() {
}

InterpretResult interpret(Chunk* chunk) {
	vm.chunk = chunk;
	vm.ip = vm.chunk->code;
	return run();
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

	while(1) {
#ifdef DEBUG_TRACE_EXECUTION
		disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
			case OP_CONSTANT: {                
				Value constant = READ_CONSTANT();
				printValue(constant);
				printf("\n");
				break;
			}
			case OP_RETURN:
				return INTERPRET_OK;
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
}
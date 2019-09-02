#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	chunk->lines = NULL;
	initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
	if (chunk->capacity < chunk->count + 1) {
		int oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code = GROW_ARRAY(chunk->code, uint8_t, oldCapacity, chunk->capacity);
		chunk->lines = GROW_ARRAY(chunk->lines, int, oldCapacity, chunk->capacity);
	}

	chunk->code[chunk->count] = byte;
	chunk->lines[chunk->count] = line;
	chunk->count++;
}

void freeChunk(Chunk* chunk) {
freeValueArray(&chunk->constants);
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);
	initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value) {
	writeValueArray(&chunk->constants, value);
	return chunk->constants.count - 1;
}
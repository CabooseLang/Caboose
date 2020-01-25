#ifndef caboose_debug_h
#define caboose_debug_h

#include "chunk.h"

void
disassembleChunk(Chunk* chunk, const char* name);
int
disassembleInstruction(Chunk* chunk, int offset);

#endif

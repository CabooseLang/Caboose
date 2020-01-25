#ifndef caboose_compiler_h
#define caboose_compiler_h

#include "chunk.h"
#include "common.h"
#include "object.h"

ObjFunction*
compile(const char* source);

void
markCompilerRoots();

#endif

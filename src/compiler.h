#ifndef caboose_compiler_h
#define caboose_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction* compile(const char* source);
void grayCompilerRoots();

#endif

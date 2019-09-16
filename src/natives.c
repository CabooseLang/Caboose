#include "natives.h"
#include "memory.h"
#include "vm.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void defineNative(const char *name, NativeFn function) {
	push(OBJ_VAL(copyString(name, (int)strlen(name))));
	push(OBJ_VAL(newNative(function)));
	tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
	pop();
	pop();
}

static void defineNativeVoid(const char *name, NativeFnVoid function) {
	push(OBJ_VAL(copyString(name, (int)strlen(name))));
	push(OBJ_VAL(newNativeVoid(function)));
	tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
	pop();
	pop();
}

// Native functions
static Value timeNative(int argCount, Value *args) {
	return NUMBER_VAL((double)time(NULL));
}

static Value clockNative(int argCount, Value *args) {
	return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value inputNative(int argCount, Value *args) {
	if (argCount > 1) {
		runtimeError("input() takes exactly 1 argument (%d given).", argCount);
		return NIL_VAL;
	}

	if (argCount != 0) {
		Value prompt = args[0];
		if (!IS_STRING(prompt)) {
			runtimeError("input() only takes a string argument!");
			return NIL_VAL;
		}

		printf("%s", AS_CSTRING(prompt));
	}

	uint8_t len_max = 128;
	uint8_t current_size = len_max;

	char *line = malloc(len_max);

	if (line == NULL) {
		runtimeError("Memory error!");
		return NIL_VAL;
	}

	int c = EOF;
	uint8_t i = 0;
	while ((c = getchar()) != '\n' && c != EOF) {
		line[i++] = (char)c;

		if (i == current_size) {
			current_size = i + len_max;
			line = realloc(line, current_size);
		}
	}

	line[i] = '\0';

	Value l = OBJ_VAL(copyString(line, strlen(line)));
	free(line);

	return l;
}

static bool printNative(int argCount, Value *args) {
	for (int i = 0; i < argCount; ++i) {
		Value value = args[i];

		if (IS_STRING(value))
			printf("%s", AS_CSTRING(value));
		else
			printValue(value);
	}

	return true;
}

static bool printlnNative(int argCount, Value *args) {
	for (int i = 0; i < argCount; ++i) {
		Value value = args[i];

		if (IS_STRING(value))
			printf("%s\n", AS_CSTRING(value));
		else {
			printValue(value);
			printf("\n");
		}
	}

	return true;
}

static bool exitNative(int argCount, Value *args) {
	if (argCount > 1) {
		runtimeError("input() takes exactly 1 argument (%d given).", argCount);
		return NIL_VAL;
	}

	if (argCount != 0) {
		Value exitCode = args[0];
		if (!IS_NUMBER(exitCode)) {
			runtimeError("exit() only takes a number argument!");
			return NIL_VAL;
		}

		exit((int)AS_NUMBER(exitCode));
	}
}

static Value randomNative(int argCount, Value *args) {
	srand(time(NULL));
	return NUMBER_VAL((double)rand() / (double)RAND_MAX);
}

static Value floorNative(int argCount, Value *args) {
	if (argCount != 1) {
		runtimeError("floor() takes exactly 1 argument (%d given).", argCount);
		return NIL_VAL;
	}

	if (!IS_NUMBER(args[0])) {
		runtimeError("floor() only takes a number value!");
		return NIL_VAL;
	}

	return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

static Value ceilNative(int argCount, Value *args) {
	if (argCount != 1) {
		runtimeError("ceil() takes exactly 1 argument (%d given).", argCount);
		return NIL_VAL;
	}

	if (!IS_NUMBER(args[0])) {
		runtimeError("ceil() only takes a number value!");
		return NIL_VAL;
	}

	return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

static Value boolNative(int argCount, Value *args) {
	if (argCount != 1) {
		runtimeError("bool() takes exactly 1 argument (%d given).", argCount);
		return NIL_VAL;
	}

	return BOOL_VAL(!isFalsey(args[0]));
}

static Value numNative(int argCount, Value *args) {
	if (argCount != 1) {
		runtimeError("number() takes exactly 1 argument (%d given).", argCount);
		return NIL_VAL;
	}

	if (!IS_STRING(args[0])) {
		runtimeError("number() only takes a string as an argument");
		return NIL_VAL;
	}

	char *numberString = AS_CSTRING(args[0]);
	double number = strtod(numberString, NULL);

	return NUMBER_VAL(number);
}

static Value strNative(int argCount, Value *args) {
	if (argCount != 1) {
		runtimeError("str() takes exactly 1 argument (%d given).", argCount);
		return NIL_VAL;
	}

	if (!IS_STRING(args[0])) {
		char *valueString = valueToString(args[0]);

		ObjString *string = copyString(valueString, strlen(valueString));
		free(valueString);

		return OBJ_VAL(string);
	}

	return args[0];
}

static Value powNative(int argCount, Value *args) {
	if (argCount != 2) {
		runtimeError("pow() takes exactly 2 arguments. (%d given)", argCount);
		return NIL_VAL;
	}

	int a = AS_NUMBER(args[0]);
	int b = AS_NUMBER(args[1]);

	return NUMBER_VAL(pow(a, b));
}

static Value lenNative(int argCount, Value* args) {
	if (argCount != 1) {
        runtimeError("len() takes exactly 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (IS_STRING(args[0])) return NUMBER_VAL(AS_STRING(args[0])->length);
    else if (IS_LIST(args[0])) return NUMBER_VAL(AS_LIST(args[0])->values.count);
    else if (IS_DICT(args[0])) return NUMBER_VAL(AS_DICT(args[0])->count);

    runtimeError("Unsupported type passed to len()", argCount);
    return NIL_VAL;
}

void defineAllNatives() {
	char *nativeNames[] = {
		"clock", "time", "input", "random", "ceil",
		"floor", "bool", "num",   "str",	"pow", "len"
	};

	NativeFn nativeFunctions[] = {
		clockNative, timeNative, inputNative, randomNative, ceilNative,
		floorNative, boolNative, numNative,   strNative,	powNative, lenNative,
	};

	char *nativeVoidNames[] = {
		"print",
		"println",
		"exit",
	};

	NativeFnVoid nativeVoidFunctions[] = {
		printNative,
		printlnNative,
		exitNative,
	};

	for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i)
		defineNative(nativeNames[i], nativeFunctions[i]);

	for (uint8_t i = 0;
		 i < sizeof(nativeVoidNames) / sizeof(nativeVoidNames[0]); ++i)
		defineNativeVoid(nativeVoidNames[i], nativeVoidFunctions[i]);
}

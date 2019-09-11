#define USE_UTF8

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "linenoise/linenoise.h"
#include "util.h"
#include "vm.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void sigintHandler(int sig) {
	(void)sig;
	printf("\nGoodbye.");
	exit(0);
}

static void repl() {
	printf("Caboose Prompt\n");
	signal(SIGINT, sigintHandler);

    linenoiseHistoryLoad(".caboose_history.cb");
	
    char *line;
	while ((line = linenoise("[>>] ")) != NULL) {
		interpret(line);

        linenoiseHistoryAdd(line);
        linenoiseHistorySave(".caboose_history.cb");
	}
}

static void runFile(const char *path) {
	char *source = readFile(path);
	InterpretResult result = interpret(source);
	free(source);

	if (result == INTERPRET_COMPILE_ERROR)
		exit(65);
	if (result == INTERPRET_RUNTIME_ERROR)
		exit(70);
}

int main(int argc, const char *argv[]) {
	initVM(argc == 1, argc == 2 ? argv[1] : "repl");

	if (argc == 1)
		repl();
	else if (argc == 2)
		runFile(argv[1]);
	else {
		fprintf(stderr, "Usage: caboose [path]\n");
		exit(64);
	}

	freeVM();
	return 0;
}

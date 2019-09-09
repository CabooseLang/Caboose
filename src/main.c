#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include "util.h"

static void sigintHandler(int sig) {
    (void)sig;
    printf("\nGoodbye.");
    exit(0);
}

static void repl() {
    printf("Caboose Prompt");
    signal(SIGINT, sigintHandler);
    char line[1024];
    while (true) {
        printf("[>>] ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static void runFile(const char *path) {
    char *source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char *argv[]) {
    initVM(argc == 1, argc == 2 ? argv[1] : "repl");

    if (argc == 1) repl();
    else if (argc == 2) runFile(argv[1]);
    else {
        fprintf(stderr, "Usage: caboose [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}

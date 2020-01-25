#include "../util.h"
#include "../vm.h"
#include <stdio.h>
#include <stdlib.h>

static void
repl() {
    printf("Caboose Prompt\n");
    char line[1024];
    for (;;) {
        printf("[>] ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static void
runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);
}

int
main(int argc, const char** argv) {
    initVM();

    if (argc == 1)
        repl();
    else if (argc == 2)
        runFile(argv[1]);
    else {
        fprintf(stderr, "Usage: cb [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}

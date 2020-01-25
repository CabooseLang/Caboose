#!/usr/bin/env bash

# Format all C files in mainSRC dir.
clang-format -style=file ./src/*.c -i
clang-format -style=file ./src/*.h -i

# Format main entrypoint file
clang-format -style=file ./src/app/main.c -i
# Caboose

> Caboose is a simple, dynamically typed, bytecode-based interpreted language built on top of a powerful VM.

Caboose aims to be a simple and easy to learn language while still being powerful enough for everyday use. It can compile anywhere C can run so give it a go!

## Building

the easiest way to build the command line tool and library is to run the following commands in the project's base directory:

```bash
cmake . -Bbuild
cmake --build ./build
```

> **Note:** This does require CMake to be installed and on your system path. If you get an error about a minimum required version, just upgrade CMake from the latest package, which can be found on their download page.

## Examples
### CLI Usage - File
Suppose you have an example Caboose file named `main.cb`:
```cb
// main.cb
fun hello(name) {
    print "hello, " + name;
}

hello("world");
```

You can run it using the caboose interpreter like so:
```bash
$ cb main.cb
```

### CLI Usage - REPL
To get going quickly, you may want to start a REPL, to do this:
```bash
$ cb
```

### Embedding
Below is a minimal embedded Caboose interpreter in C
```c
#include <caboose/vm.h>

int main() {
    initVM();

    InterpretResult result = interpret("var some = \"example source code\";");
    
    // These exit codes represent the closest thing in Unix to what they actually mean in this context
    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);

    freeVM();
}
```

## License

Caboose is licensed under the [MIT License](LICENSE).
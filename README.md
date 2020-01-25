# Caboose

> Caboose is a simple, dynamically typed, bytecode-based interpreted language built on top of a powerful VM.

Caboose aims to be a simple and easy to learn language while still being powerful enough for everyday use. It can compile anywhere C can run so give it a go!

## Building

the easiest way to build the command line tool and shared library is to run the following commands in the project's base directory:

```bash
cmake . -Bcmake-build-release
cmake --build ./cmake-build-release
```

> **Note:** This does require CMake to be installed and on your system path. If you get an error about a minimum required version, just upgrade CMake from the latest package, which can be found on their download page.

---
description: Let's get started using caboose!
---

# Getting Started

## Building the Compiler

Caboose is fairly new so we haven't had the time to package it for different OSes yet, therefore you will have to build it from source.

```bash
$ git clone https://github.com/CabooseLang/Caboose.git
$ cd Caboose
$ bash build.sh # Uses CMake to build the executable
```

{% hint style="info" %}
 Building requires the latest CMake on your system path.
{% endhint %}

To run some Caboose code:

```bash
$ ./build/caboose # For the REPL
$ ./build/caboose /path/to/your/code.cb # For a file
```




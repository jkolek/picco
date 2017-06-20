# PICCO (PIco C COmpiler) version 0.9 (under development)


PICCO is an ANSI C compiler, that comes together with tools such as linker,
disassembler and emulator. It has its own ABI and linkable/executable object
format. The compiler and the linker are written in
C++, emulators are written in C.


## The Software Tools

`picco` - ANSI C compiler. It generates a linkable object or assembly file for
one of the supported target platforms.

`plinker` - static linker for PICCO specific object files.

`armv7emu` - ARMv7 emulator.


## Installation:

To download and install PICCO, type into the terminal:

```
  $ git clone https://github.com/jkolek/picco
  $ cd picco
  $ make
  $ sudo make install
```

This will install `picco`, `plinker` and emulator executables into directory
`/usr/bin`.

To run tests, in picco source directory type:

```
  $ python runtests.py
```

To clean up, just type:

```
  $ make cleanup
```

Usage example:

```
  $ picco test/c/scheme.c --target arm -o scheme.o
  $ plinker scheme.o -o scheme.out
  $ armv7emu scheme.out
```


## The Compiler Structure and Data Flow

The compiler has hand coded top-down recursive descent parser, which generates
an abstract syntax tree (AST) and a symbol table. Then AST is traversed and
intermediate representation (IR) is being generated. Finally, a machine
dependent code is generated from the IR.

This is the general structure and data flow of the compiler:

```
       +-----------------+
       |     hello.c     |
       +-----------------+
               ||
               \/
  +===================================================+
  | PICCO                                             |
  |                                                   |
  |   +===================+                           |
  |   |      C lexer      |                           |
  |   +===================+                           |
  |            ||                                     |
  |            \/                                     |
  |    +-----------------+                            |
  |    |      Tokens     |                            |
  |    +-----------------+                            |
  |            ||                                     |
  |            \/                                     |
  |   +===================+     +-----------------+   |
  |   |     C parser      | ==> |  Symbol table   |   |
  |   +===================+     +-----------------+   |
  |            ||                                     |
  |            \/                                     |
  |    +-----------------+                            |
  |    |      AST        |                            |
  |    +-----------------+                            |
  |            ||                                     |
  |            \/                                     |
  |   +===================+     +-----------------+   |
  |   |   AST traverser   | <== |  Symbol table   |   |
  |   +===================+     +-----------------+   |
  |            ||                                     |
  |            \/                                     |
  |    +-----------------+                            |
  |    |       IR        |                            |
  |    +-----------------+                            |
  |            ||                                     |
  |            \/                                     |
  |   +===================+                           |
  |   |   IR traverser    |                           |
  |   +===================+                           |
  |            ||                                     |
  |            \/                                     |
  |    +-----------------+                            |
  |    |     Items       |                            |
  |    +-----------------+                            |
  |            ||                                     |
  |            \/                                     |
  |   +===================+                           |
  |   |   Code generator  |                           |
  |   +===================+                           |
  |                                                   |
  +===================================================+
               ||
               \/
       +-----------------+
       |     hello.o     |  (Linkable object file)
       +-----------------+
               ||
               \/
  +===========================+
  |           Linker          |
  +===========================+
               ||
               \/
       +-----------------+
       |     hello.out   |  (Executable file)
       +-----------------+
```

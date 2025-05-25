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

This will install the executables `picco`, `plinker` and `armv7emu` into
directory `/usr/bin`.

To run tests, in the picco test directory type:

```
  $ python runtests.py
```

To clean up, just type:

```
  $ make cleanup
```

Usage example:

```
  $ picco test/c/add_nums.c --target arm # The output will be output.o
  $ plinker output.o --target arm        # The output will be output.out
  $ armv7emu output.out
  $ echo $?
```

To dump an abstract syntax tree, IR expression tree or target machine code of the
compiled source, append option `-t`, `-e` or `-d` respectivelly when calling
`picco`:

```
  $ picco test/c/add_nums.c --target arm -t

Abstract syntax tree:

    NK_LIST
    Element 0:
        NK_FUNCTION_DECL
        Function name:
            NK_IDENT_NODE
            Value: main
        Type:
            NK_INTEGRAL_TYPE
            Alignment: 4
            Signed: true
        Body:
            NK_COMPOUND_STMT
        ...


  $ picco test/c/add_nums.c --target arm -e

IR expression tree:

    (seq
        (function main [16]
            (seq
                (move i32
                    (mem i32
                        (binop plus i32
                            (temp SP)
                            (const i32 )
                        )
                    )
                    (const i32 )
                )
    ...


  $ picco test/c/add_nums.c --target arm -t

Symbol table section:

  Symbol 0 {
    Name:           main
    Offset:         0
  }

Relocation section:

Code buffer:

 0:     e24dd010    sub	sp, sp, #16
 4:     e3a00016    mov	r0, #22
 8:     e78d000c    str	r0, [sp, #12]
 12:    e3a00021    mov	r0, #33
 16:    e78d0008    str	r0, [sp, #8]
 20:    e79d000c    ldr	r0, [sp, #12]
 24:    e79d1008    ldr	r1, [sp, #8]
 28:    e0800001    add	r0, r0, r1
 32:    e78d0004    str	r0, [sp, #4]
 36:    e79d0004    ldr	r0, [sp, #4]
 40:    e28dd010    add	sp, sp, #16
 44:    e1a0f00e    mov	pc, lr
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

The compiler is modular, there are five parts that can be treated independently:
  - Lexer
  - Parser
  - AST Traverser
  - IR Traverser
  - Code Generator

For example the lexer and the parser can be reused in some other programs.

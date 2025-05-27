// PICCO main driver code.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../include/Parser.h"
#include "../include/IRExprTree.h"

#define VERSION "0.9"

#define INFO_STR                                                               \
    "picco (Pico C Compiler) " VERSION "\n"                                    \
    "Copyright (C) 2017 Jozef Kolek  <jkolek@gmail.com>. "                     \
    "All Rights Reserved.\n\n"

#define HELP_STR                                                               \
    "INPUT and OUTPUT stands for input and output files respectively\n\n"      \
    "  -o, --output <file>      Output object file, if not specified, "        \
    "default\n"                                                                \
    "                           'output.out' file is generated\n"              \
    "  -S,                      Output assembly code\n"                        \
    "  -d, --disassemble        Print out code buffer\n"                       \
    "  -t, --print-tree         Print out abstract syntax tree\n"              \
    "  -e, --print-expr-tree    Print out intermediate representation\n"       \
    "  --target <value>         Generate code for the given target\n"          \
    "  -h, --help               Print out this help information\n"             \
    "  -v, --version            Print out only version information\n\n"

void print_info() { std::cout << INFO_STR; }

void print_usage(const char *progname)
{
    std::cout << "Usage: picco INPUT [-o OUTPUT] [-S] [-d] [-t] [-e] [-h] [-v]"
              << std::endl;
}

void print_help() { std::cout << HELP_STR; }

// Picco facade class
class PiccoFacade
{
public:
    PiccoFacade() {}

    void compile(char *input,
                 char *output,
                 char *target,
                 bool printAST,
                 bool printIR,
                 bool printCFG,
                 bool printCodeBuf,
                 bool optimize,
                 bool asmOutput)
    {
        std::cout << "Compiling: \"" << input << "\"\n";

        CLexer lexer(input);
        CParser parser(&lexer, target);
        parser.parse(output, optimize);

        AbstractSyntaxTree *ast = parser.getAST();

        CodeGenerator *codegen = ast->getCodeGenerator();
        if (asmOutput)
            codegen->setAsmOutput(true);

        ASTNode *root = ast->getRoot();
        ListASTNode *translationUnit = static_cast<ListASTNode *>(root);

        IRExprTree *iret = ast->getIRExprTree();
        IRExpr *rootIR = ast->emitIR(translationUnit);

        // TODO: Register passes methods and use loop to iterate over them.

        // Jump optimization phase.
        iret->jumps(rootIR);
        iret->setPreserveReturnValueToFalse(rootIR);
        // iret->removeUnusedLabels(rootIR);

        iret->setRoot(rootIR);

        // FIXME: This function causes compiler to emit jump to 'main_epilogue'
        // for test 'while.c' even if 'main_epilogue' doesn't exists.
        // iret->removeLabelFollowedByLabel(rootIR);

        // TODO: Create IR tree from the basic blocks.
        //if (optimize)
        //    iret->controlFlowAnalysis(rootIR);

        // Emit machine code to the buffer.
        iret->emitCode(rootIR);

        //if (errors == 0)
        codegen->write(output);

        if (!ast->getWarnings())
            std::cout << "No errors; no warnings; lines: "
                      << parser.getCurrentLine() << std::endl;
        else
            std::cout << "No errors; warnings: " << ast->getWarnings()
                      << "; lines: " << parser.getCurrentLine() << std::endl;

        if (printAST)
            // TODO: Dump AST to a file.
            ast->printTree();

        if (printIR)
            // TODO: Dump IR to a file.
            iret->printIRExprTree();

        if (printCodeBuf)
            // TODO: Dump code to a file.
            codegen->decode();
    }
};

int main(int argc, char **argv)
{
    char *input;
    char output[256], target[256];
    bool outputOk, printHelp, printVersion, printAST, printIR, printCodeBuf,
        printCFG, optimize, asmOutput;
    int n;

    if (argc <= 1)
    {
        print_info();
        print_usage(argv[0]);
        std::cout << "Try -h option for more info." << std::endl;
        exit(1);
    }

    // Default target is ARMv7
    strcpy(target, "arm");
    input = NULL;
    printHelp = false;
    printVersion = false;
    outputOk = false;
    printAST = false;
    printIR = false;
    printCodeBuf = false;
    printCFG = false;
    optimize = false;
    asmOutput = false;
    n = 1;

    while (n < argc)
    {
        if (strcmp(argv[n], "-o") == 0 || strcmp(argv[n], "--output") == 0)
        {
            strcpy(output, argv[++n]);
            outputOk = true;
        }
        else if (strcmp(argv[n], "-d") == 0 ||
                 strcmp(argv[n], "--disassemble") == 0)
        {
            printCodeBuf = true;
        }
        else if (strcmp(argv[n], "-t") == 0 ||
                 strcmp(argv[n], "--print-tree") == 0)
        {
            printAST = true;
        }
        else if (strcmp(argv[n], "-e") == 0 ||
                 strcmp(argv[n], "--print-expr-tree") == 0)
        {
            printIR = true;
        }
        else if (strcmp(argv[n], "--target") == 0)
        {
            strcpy(target, argv[++n]);
        }
        else if (strcmp(argv[n], "--optimize") == 0)
        {
            optimize = true;
        }
        else if (strcmp(argv[n], "-S") == 0)
        {
            asmOutput = true;
        }
        else if (strcmp(argv[n], "-h") == 0 || strcmp(argv[n], "--help") == 0)
        {
            printHelp = true;
        }
        else if (strcmp(argv[n], "-v") == 0 ||
                 strcmp(argv[n], "--version") == 0)
        {
            printVersion = true;
        }
        else
        {
            input = argv[n];
        }
        n++;
    }

    if (printHelp)
    {
        print_info();
        print_usage(argv[0]);
        print_help();
        exit(0);
    }

    if (printVersion)
    {
        print_info();
        exit(0);
    }

    if (input == NULL)
    {
        std::cerr << "picco: fatal error: no input file" << std::endl;
        exit(1);
    }

    if (!outputOk)
    {
        if (asmOutput)
            strcpy(output, "output.s");
        else
            strcpy(output, "output.o");
    }

    PiccoFacade picco;

    picco.compile(input, output, target, printAST, printIR, printCFG,
                  printCodeBuf, optimize, asmOutput);

    exit(0);
}

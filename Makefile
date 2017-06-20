OBJS = main.o Parser.o SymbolTable.o CodeGenerator.o ARMCodeGenerator.o \
       Intel8086CodeGenerator.o Lexer.o \
       AbstractSyntaxTree.o ASTNode.o PrintTreeVisitor.o TreeVisitor.o \
       IRExprTree.o ELFObject.o IRExprVisitor.o IRExpr.o \
       ARMDecoder.o ControlFlowGraph.o

CXX = g++
CC  = gcc
CXXFLAGS = -g -std=c++11 #-Wall #-D GNU_ABI
CFLAGS = -g

SRC = src
INCLUDE = include
BIN = .
DEST = /usr/bin

all: picco plinker armv7emu

picco: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o picco

main.o: ${SRC}/main.cpp ${INCLUDE}/Parser.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/main.cpp

Parser.o: ${INCLUDE}/Parser.h ${INCLUDE}/Lexer.h ${INCLUDE}/SymbolTable.h \
 ${INCLUDE}/ARMCodeGenerator.h \
 ${INCLUDE}/AbstractSyntaxTree.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/Parser.cpp

CodeGenerator.o: ${INCLUDE}/CodeGenerator.h ${INCLUDE}/PiccoObjectFormat.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/CodeGenerator.cpp

ARMCodeGenerator.o: ${INCLUDE}/CodeGenerator.h ${INCLUDE}/ARMCodeGenerator.h \
 ${INCLUDE}/Decoder.h ${INCLUDE}/SymbolTable.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/ARMCodeGenerator.cpp

ARMDecoder.o: ${INCLUDE}/Decoder.h ${INCLUDE}/ARMDecoder.h \
 ${INCLUDE}/Decoder.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/ARMDecoder.cpp

Intel8086CodeGenerator.o: ${INCLUDE}/CodeGenerator.h \
 ${INCLUDE}/Intel8086CodeGenerator.h ${INCLUDE}/Decoder.h \
 ${INCLUDE}/SymbolTable.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/Intel8086CodeGenerator.cpp

SymbolTable.o: ${INCLUDE}/SymbolTable.h ${INCLUDE}/common.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/SymbolTable.cpp

Lexer.o: ${INCLUDE}/Lexer.h ${INCLUDE}/common.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/Lexer.cpp

AbstractSyntaxTree.o: ${INCLUDE}/AbstractSyntaxTree.h \
 ${INCLUDE}/CodeGenerator.h ${INCLUDE}/ARMCodeGenerator.h \
 ${INCLUDE}/ASTNode.h ${INCLUDE}/TreeVisitor.h \
 ${INCLUDE}/PrintTreeVisitor.h ${INCLUDE}/IRExprVisitor.h \
 ${INCLUDE}/PrintIRExprVisitor.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/AbstractSyntaxTree.cpp

PrintTreeVisitor.o: ${INCLUDE}/ASTNode.h ${INCLUDE}/TreeVisitor.h \
 ${INCLUDE}/PrintTreeVisitor.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/PrintTreeVisitor.cpp

#PrintIRExprVisitor.o: IRExpr.h IRExprVisitor.h PrintIRExprVisitor.h
#	$(CXX) $(CXXFLAGS) -c PrintIRExprVisitor.cpp

TreeVisitor.o: ${INCLUDE}/ASTNode.h ${INCLUDE}/TreeVisitor.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/TreeVisitor.cpp

IRExprVisitor.o: ${INCLUDE}/IRExpr.h ${INCLUDE}/IRExprVisitor.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/IRExprVisitor.cpp

ASTNode.o: ${INCLUDE}/TreeVisitor.h ${INCLUDE}/ASTNode.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/ASTNode.cpp

IRExpr.o: ${INCLUDE}/IRExprVisitor.h ${INCLUDE}/IRExpr.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/IRExpr.cpp

IRExprTree.o: ${INCLUDE}/IRExprTree.h ${INCLUDE}/CodeGenerator.h \
  ${INCLUDE}/ASTNode.h ${INCLUDE}/TreeVisitor.h \
  ${INCLUDE}/PrintTreeVisitor.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/IRExprTree.cpp

ControlFlowGraph.o: ${INCLUDE}/IRExpr.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/ControlFlowGraph.cpp

ELFObject.o: ${INCLUDE}/ELFObject.h ${INCLUDE}/ELF.h
	$(CXX) $(CXXFLAGS) -c ${SRC}/ELFObject.cpp

plinker:
	$(CXX) $(CXXFLAGS) ${SRC}/Linker.cpp ${INCLUDE}/PiccoObjectFormat.h -o plinker

armv7emu:
	$(CC) $(CFLAGS) ${SRC}/armv7emu.c ${INCLUDE}/PiccoObjectFormat.h -o armv7emu

install:
	-cp ${BIN}/picco ${DEST}

clean:
	-rm *.o picco plinker armv7emu

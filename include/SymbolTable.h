// Symbol table - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "common.h"
#include <map>
#include <vector>

// Object kinds
enum ObjectKind
{
    OBJ_CON,
    OBJ_VAR,
    OBJ_VAR_STATIC,
    OBJ_TYPE,
    OBJ_FUNC,
    OBJ_PAR,
    OBJ_LAB,
    OBJ_PTR
};

// Type kinds
enum TypeKind
{
    T_NONE,
    T_CHAR,
    T_SHORT,
    T_INT,
    T_UNSIGNED,
    T_LONG,
    T_FLOAT,
    T_DOUBLE,
    T_VOID,
    T_ARRAY,
    T_STRUCT,
    T_UNION,
    T_BOOL,
    T_REAL,
    T_POINTER,
    T_ENUM,
    T_FUNCTION // Can we have T_FUNC_PTR instead,
               // to merge T_POINTER and T_FUNCTION?
};

struct Type;
struct Object;
struct Scope;

typedef struct Type
{
    TypeKind kind;
    // TODO: Put elemType, baseType and funcType in union
    struct Type *elemType; // Array element type
    struct Type *baseType; // Base type of pointer type
    struct Type *funcType; // Function pointer type
    int nFields;             // Number of struct fields
    int length;              // Array length
    struct Object *fields; // Struct fields
    int size;
    bool isSigned;

    Type(TypeKind Kind)
        : elemType(nullptr), baseType(nullptr), funcType(nullptr), nFields(0),
          length(0), fields(nullptr), size(0), isSigned(false)
    {
        kind = Kind;
    }
} * Type_t;

typedef struct Object
{
    ObjectKind kind;
    char name[IDLEN];
    Type_t type;
    bool inRegister; // True if the variable is in register instead of on stack.
    unsigned reg;    // If inRegister is true than reg contains the register in
                     // which variable is stored.
    int parIndex;
    int used;
    int ival;  // Integer constant value.
    int adr;   // Function address or local variable GP/SP relative offset.
    int level; // Scope level

    // Function specific attributes
    unsigned prmc; // Function parameter count
    int frameSize; // Function frame size in bytes
    int containsFunctionCall;
    struct Object *locals; // Function parameters and local variables

    // Name of function whose this static variable is. If holds level > -1 then
    // then this field is unused.
    char staticVarFuncName[IDLEN];

    bool isConstant;

    // Next object in a list
    struct Object *next;

    Object(const char *Name, ObjectKind Kind, Type_t Type)
        : kind(Kind), type(Type), inRegister(false), reg(0), parIndex(0),
          used(false), ival(0), adr(0), level(0),
          prmc(0), frameSize(0), containsFunctionCall(false),
          locals(nullptr), isConstant(false), next(nullptr)
    {
        strcpy(name, Name);
    }
} * Object_t;

typedef struct Scope
{
    struct Scope *outer; // Pointer to the next outer scope
    Object_t locals;     // Pointer to the objects in this scope
    int nVars;           // Number of variables in this scope
    int nPars;           // Number of variables in this scope
    int size;            // Size of scope in bytes

    Scope() : outer(nullptr), locals(nullptr), nVars(0), nPars(0), size(0) {}
} * Scope_t;

class SymbolTable
{
    Scope_t topScope;    // Current scope
    Scope_t globalScope; // Current scope
    int level;           // (0 = global, 1 >= local)
    int temporaryCount;
    std::vector<Scope_t> scopePool;
    std::vector<Object_t> objectPool;
    std::vector<Type_t> typePool;

public:
    // Predefined types
    Type_t charType;
    Type_t shortType;
    Type_t intType;
    Type_t unsignedType;
    Type_t longType;
    Type_t floatType;
    Type_t doubleType;
    Type_t voidType;
    Type_t nullType;
    Type_t noType;

    Object_t noObj;

    // Allocators
    Type_t allocType(TypeKind kind);
    Object_t allocObject(const char *, ObjectKind, Type_t);
    Scope_t allocScope();

    // Symbol table interface
    Object_t insert(const char *, ObjectKind, Type_t);
    Object_t insertGlobalVariable(const char *, Type_t, const char *);
    Object_t getNewTemporary(Type_t);
    void insertFields(Type_t, char **, int, Type_t);
    Object_t getParameter(unsigned i);
    Object_t find(const char *);
    Object_t findEnum(const char *);
    Object_t findField(const char *, Type_t);

    int getLevel() { return level; }

    void openScope(void);
    void closeScope(void);
    void setTopScope(Scope_t s);
    const Scope_t getTopScope();

    void printAddresses();
    void printScopes();

    bool isIntegralType(Type_t);
    bool isRealType(Type_t);
    bool isArithmeticType(Type_t);
    bool isPointerType(Type_t);
    bool isScalarType(Type_t);
    bool isFunctionPointerType(Type_t);
    bool equalFunctionPointerTypes(Type_t, Type_t);
    bool equalBasePointerTypes(Type_t, Type_t);

    bool compatible(Type_t, Type_t);
    bool assignable(Type_t, Type_t);
    bool convertible(Type_t, Type_t);

    SymbolTable();
    ~SymbolTable();
};

#endif

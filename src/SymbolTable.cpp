// Symbol table - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include "../include/SymbolTable.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

Type_t SymbolTable::allocType(TypeKind kind)
{
    Type_t type = new Type(kind);
    typePool.push_back(type);
    return type;
}

Object_t
SymbolTable::allocObject(const char *name, ObjectKind kind, Type_t type)
{
    Object_t obj = new Object(name, kind, type);
    objectPool.push_back(obj);
    return obj;
}

Scope_t SymbolTable::allocScope()
{
    Scope_t scope = new Scope();
    scopePool.push_back(scope);
    return scope;
}

SymbolTable::SymbolTable()
{
    level = -1;
    topScope = allocScope();
    topScope->outer = nullptr;
    globalScope = topScope;

    // Built-in types

    // char: 1 byte
    charType = allocType(T_CHAR);
    charType->size = 1;
    charType->isSigned = true;

    // char: 2 bytes
    shortType = allocType(T_SHORT);
    shortType->size = 2;
    shortType->isSigned = true;

    // int: 4 bytes
    intType = allocType(T_INT);
    intType->size = 4;
    intType->isSigned = true;

    // int: 4 bytes
    unsignedType = allocType(T_UNSIGNED);
    unsignedType->size = 4;
    unsignedType->isSigned = false;

    // int: 4 bytes
    longType = allocType(T_LONG);
    longType->size = 4;
    longType->isSigned = true;

    // float: 4 bytes
    floatType = allocType(T_FLOAT);
    floatType->size = 4;

    // double: 8 bytes
    doubleType = allocType(T_DOUBLE);
    doubleType->size = 8;

    // int: 4 bytes
    voidType = allocType(T_VOID);
    voidType->size = 4;

    noType = allocType(T_NONE);
    insert("NOTYPE", OBJ_TYPE, noType);

    nullType = allocType(T_POINTER);

    // Dummy object
    noObj = allocObject("noObj", OBJ_VAR, noType);
}

SymbolTable::~SymbolTable()
{
    for (unsigned i = 0; i < scopePool.size(); i++)
        free((Scope_t)scopePool[i]);
    for (unsigned i = 0; i < objectPool.size(); i++)
        free((Object_t)objectPool[i]);
    for (unsigned i = 0; i < typePool.size(); i++)
        free((Type_t)typePool[i]);
}

Object_t SymbolTable::insert(const char *name, ObjectKind kind, Type_t type)
{
    // Create object node
    Object_t obj = allocObject(name, kind, type);
    Object_t p = nullptr, last = nullptr;

    obj->adr = topScope->size;
    obj->inRegister = false;

    if (kind == OBJ_VAR)
    {
        unsigned varSize = type->size;

        if (varSize % 4)
            varSize += 4 - (varSize % 4);
        topScope->size += varSize;
        topScope->nVars++;
        obj->level = level;
    }
    else if (kind == OBJ_PAR)
    {
        unsigned parSize = type->size;

        if (parSize % 4)
            parSize += 4 - (parSize % 4);
        topScope->size += parSize;
        obj->parIndex = topScope->nPars;
        topScope->nPars++;
        obj->level = level;
    }

    // Append object node
    p = topScope->locals;
    last = nullptr;
    while (p != nullptr)
    {
        if (strcmp(p->name, name) == 0)
            return noObj;
        last = p;
        p = p->next;
    }

    if (last == nullptr)
        topScope->locals = obj;
    else
        last->next = obj;

    return obj;
}

Object_t SymbolTable::insertGlobalVariable(const char *name,
                                           Type_t type,
                                           const char *funcName)
{
    // Create object node
    Object_t obj = allocObject(name, OBJ_VAR_STATIC, type);
    Object_t p = nullptr, last = nullptr;
    unsigned varSize = type->size;

    obj->adr = globalScope->size;

    if (varSize % 4)
        varSize += 4 - (varSize % 4);
    globalScope->size += varSize;
    globalScope->nVars++;
    obj->level = level;

    if (funcName != nullptr)
        strcpy(obj->staticVarFuncName, funcName);

    // Append object node
    p = globalScope->locals;
    last = nullptr;
    while (p != nullptr)
    {
        if (strcmp(p->name, name) == 0)
            return noObj;
        last = p;
        p = p->next;
    }

    if (last == nullptr)
        globalScope->locals = obj;
    else
        last->next = obj;

    return obj;
}

Object_t SymbolTable::getNewTemporary(Type_t type)
{
    Object_t obj;
    char name[10];

    sprintf(name, "$t%d", temporaryCount);
    temporaryCount++;
    obj = insert(name, OBJ_VAR, type);

    return obj;
}

void SymbolTable::insertFields(Type_t recTp, char **idents, int n, Type_t tp)
{
    Object_t tmp;
    int i;

    if (recTp->fields != nullptr)
    {
        tmp = recTp->fields;
        while (tmp->next != nullptr)
            tmp = tmp->next;
        tmp->next = insert(idents[0], OBJ_VAR, tp);
        tmp = tmp->next;
    }
    else
    {
        recTp->fields = insert(idents[0], OBJ_VAR, tp);
        tmp = recTp->fields;
    }

    for (i = 1; i < n; i++)
    {
        tmp->next = insert(idents[i], OBJ_VAR, tp);
        tmp = tmp->next;
    }
}

Object_t SymbolTable::getParameter(unsigned i)
{
    Object_t p;
    unsigned n = 0;

    for (p = topScope->locals; p != nullptr; p = p->next)
        if (n == i)
            return p;
        else
            n++;

    return noObj;
}

Object_t SymbolTable::find(const char *name)
{
    Scope_t s;
    Object_t p;

    for (s = topScope; s != nullptr; s = s->outer)
        for (p = s->locals; p != nullptr; p = p->next)
            if (strcmp(p->name, name) == 0)
            {
                p->used = true;
                return p;
            }

    return noObj;
}

Object_t SymbolTable::findEnum(const char *name)
{
    Scope_t s;
    Object_t p, q;

    for (s = topScope; s != nullptr; s = s->outer)
        for (p = s->locals; p != nullptr; p = p->next)
        {
            if (p->kind == OBJ_TYPE && p->type->kind == T_ENUM)
            {
                Type_t type = p->type;
                for (q = type->fields; q != nullptr; q = q->next)
                {
                    if (strcmp(q->name, name) == 0)
                        return q;
                }
            }
        }

    return noObj;
}

Object_t SymbolTable::findField(const char *name, Type_t type)
{
    Object_t p;

    for (p = type->fields; p != nullptr; p = p->next)
        if (strcmp(p->name, name) == 0)
            return p;

    return noObj;
}

void SymbolTable::openScope(void)
{
    Scope_t s = allocScope();

    s->outer = topScope;
    topScope = s;
    topScope->size = 0;
    level++;
}

void SymbolTable::closeScope(void)
{
    topScope = topScope->outer;
    level--;
}

void SymbolTable::setTopScope(Scope_t s) { topScope = s; }

const Scope_t SymbolTable::getTopScope() { return topScope; }

void SymbolTable::printAddresses()
{
    Scope_t s;
    Object_t p;

    s = topScope;
    if (s != nullptr)
    {
        printf("Symtab names ---------\n");
        for (p = s->locals; p != nullptr; p = p->next)
            printf("p->name = %s, p->adr = %d\n", p->name, p->adr);
        printf("Symtab names ---------\n");
    }
}

void SymbolTable::printScopes()
{
    Scope_t s;
    Object_t p;

    for (s = topScope; s != nullptr; s = s->outer)
        for (p = s->locals; p != nullptr; p = p->next)
            printf("--- %s\n", p->name);
}

bool SymbolTable::isIntegralType(Type_t t)
{
    assert(t != nullptr && "Unexpected null pointer!");

    return (t == charType || t == intType || t == unsignedType ||
            t == shortType || t == longType);
}

bool SymbolTable::isRealType(Type_t t)
{
    assert(t != nullptr && "Unexpected null pointer!");

    return (t == floatType || t == doubleType);
}

bool SymbolTable::isArithmeticType(Type_t t)
{
    assert(t != nullptr && "Unexpected null pointer!");

    return (isIntegralType(t) || isRealType(t));
}

bool SymbolTable::isPointerType(Type_t t)
{
    assert(t != nullptr && "Unexpected null pointer!");

    return t->kind == T_POINTER;
}

bool SymbolTable::isScalarType(Type_t t)
{
    return isPointerType(t) || isArithmeticType(t);
}

bool SymbolTable::isFunctionPointerType(Type_t t)
{
    return (t->kind == T_POINTER && t->baseType->kind == T_FUNCTION);
}

bool SymbolTable::equalFunctionPointerTypes(Type_t t1, Type_t t2)
{
    // TODO: Check if parameters of the function types match.
    return (isFunctionPointerType(t1) && isFunctionPointerType(t2) &&
            (t1->baseType->funcType == t2->baseType->funcType));
}

bool SymbolTable::equalBasePointerTypes(Type_t t1, Type_t t2)
{
    return (t1->kind == T_POINTER && t2->kind == T_POINTER &&
            (t1->baseType == t2->baseType));
}

// Two pointer types with the same type qualifiers are compatible if they
// point to objects of compatible type. The composite type for two compatible
// pointer types is the similarly qualified pointer to the composite type.

bool SymbolTable::compatible(Type_t t1, Type_t t2)
{
    assert(t1 != nullptr && t2 == nullptr && "Unexpected null pointer!");

    return ((t1 == t2) || (isIntegralType(t1) && isIntegralType(t2)) ||
            (isIntegralType(t1) && t2->kind == T_POINTER) ||
            (isIntegralType(t2) && t1->kind == T_POINTER) ||
            (t1->kind == T_POINTER && t2->baseType == noType) ||
            (t2->kind == T_POINTER && t1->baseType == noType) ||
            ((t1->kind == T_POINTER && t2->kind == T_POINTER) &&
             compatible(t1->baseType, t2->baseType)));
}

bool SymbolTable::assignable(Type_t t1, Type_t t2)
{
    assert(t1 != nullptr && t2 == nullptr && "Unexpected null pointer!");

    return ((isArithmeticType(t1) && isArithmeticType(t2)) ||
            (t1->kind == T_POINTER && t2->baseType == noType) ||
            (t2->kind == T_POINTER && t1->baseType == noType) ||
            ((t1->kind == T_POINTER && t2->kind == T_POINTER) &&
             (t1->baseType == t2->baseType)));
}

bool SymbolTable::convertible(Type_t t1, Type_t t2) { return true; }

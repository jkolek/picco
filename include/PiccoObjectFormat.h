// PICCO object format, v0.1
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

struct symbol_t
{
    char name[32];
    unsigned offset;
};

struct objheader_t
{
    unsigned hflags;
    int mainPC;
    int readOnlyDataSize;
    int staticDataSize;
    unsigned symbolsc;
    unsigned relocsc;
    int codeSize;
};

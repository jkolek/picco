// Decoder base classes.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef DECODER_H
#define DECODER_H

#include "ELFObject.h"

// TODO: Implement endianess support
class Decoder
{
protected:
    unsigned PC;
    unsigned char *buf;
    Elf32_Rel *relocs;
    unsigned relocsc;
    bool isLittleEndian;

public:
    virtual const char *getRelocName(unsigned reloc) { return nullptr; }
    virtual void printBuffer() {}

    Decoder(unsigned char *BUF,
            unsigned PC0,
            Elf32_Rel *Relocs,
            unsigned Relocsc,
            bool isLE)
        : PC(PC0), buf(BUF), relocs(Relocs), relocsc(Relocsc),
          isLittleEndian(isLE)
    {
    }
    virtual ~Decoder() {}
};

#endif

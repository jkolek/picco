// ARM decoder - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef ARM_DECODER_H
#define ARM_DECODER_H

#include "Decoder.h"

class ARMDecoder : public Decoder
{
    const char *opToStr(unsigned op);
    const char *opToStrSpecial(unsigned op);
    const char *opToStrSpecial2(unsigned op);
    const char *opToStrCOP1(unsigned op);
    const char *regToStr(unsigned reg);
    const char *regListToStr(unsigned regList);
    const char *regFPToStr(unsigned reg);
    char getHexStr(unsigned int n);
    void printHexa(unsigned binary);
    void decode(unsigned binary);

    uint8_t get1b(int pc) { return buf[pc]; }

    uint16_t get2b(int pc)
    {
        // return ((get1b(pc) << 8) | get1b(pc + 1));
        return (get1b(pc) | (get1b(pc + 1) << 8));
    }

    uint32_t get4b(int pc)
    {
        // return ((get2b(pc) << 16) | get2b(pc + 2));
        return (get2b(pc) | (get2b(pc + 2) << 16));
    }

public:
    void printBuffer();
    const char *getRelocName(unsigned reloc);

    ARMDecoder(unsigned char *BUF, int PC0, Elf32_Rel *Relocs, unsigned Relocsc,
        bool isLittleEndian)
        : Decoder(BUF, PC0, Relocs, Relocsc, isLittleEndian)
    {
    }
    ~ARMDecoder() {}
};

#endif

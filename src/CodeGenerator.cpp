// Code generator implementation classes.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include "../include/CodeGenerator.h"

const unsigned CodeGenerator::inverse[6] = {GEN_NE, GEN_EQ, GEN_GE,
                                            GEN_GT, GEN_LE, GEN_LT};

// Order of bits is: 31..0
void CodeGenerator::setBits(unsigned &binary,
                            unsigned value,
                            unsigned at,
                            unsigned numbits)
{
    unsigned modby = at + numbits;

    if (modby < 31)
        binary |= ((value << at) % (0x1 << modby));
    else
        binary |= (value << at);
}

// Code buffer manipulation methods

void CodeGenerator::put1b(uint8_t x) { buf.push_back(x); PC++; }

void CodeGenerator::put2b(uint16_t x)
{
    if (isLittleEndian)
    {
        put1b(x);
        put1b(x >> 8);
    }
    else
    {
        put1b(x >> 8);
        put1b(x);
    }
}

void CodeGenerator::put4b(uint32_t x)
{
    if (isLittleEndian)
    {
        put2b(x);
        put2b(x >> 16);
    }
    else
    {
        put2b(x >> 16);
        put2b(x);
    }
}

void CodeGenerator::put1b(int pos, uint8_t x) { buf[pos] = x; }

void CodeGenerator::put2b(int pos, uint16_t x)
{
    if (isLittleEndian)
    {
        put1b(pos, x);
        put1b(pos + 1, x >> 8);
    }
    else
    {
        put1b(pos, x >> 8);
        put1b(pos + 1, x);
    }
}

void CodeGenerator::put4b(int pos, uint32_t x)
{
    if (isLittleEndian)
    {
        put2b(pos, x);
        put2b(pos + 2, x >> 16);
    }
    else
    {
        put2b(pos, x >> 16);
        put2b(pos + 2, x);
    }
}

uint8_t CodeGenerator::get1b(int pc) { return buf[pc]; }

uint16_t CodeGenerator::get2b(int pc)
{
    if (isLittleEndian)
        return (get1b(pc) | (get1b(pc + 1) << 8));

    return ((get1b(pc) << 8) | get1b(pc + 1));
}

uint32_t CodeGenerator::get4b(int pc)
{
    if (isLittleEndian)
        return (get2b(pc) | (get2b(pc + 2) << 16));

    return ((get2b(pc) << 16) | get2b(pc + 2));
}

// Helper methods

void CodeGenerator::setLower16(int pos, uint32_t value)
{
    uint32_t binary = get4b(pos);

    binary &= 0xFFFF0000;
    binary |= (value % 0x10000);
    put4b(pos, binary);
}

uint32_t CodeGenerator::getLower16(int pos) { return (get4b(pos) % 0x10000); }

void CodeGenerator::setLow24(int pos, uint32_t value)
{
    uint32_t binary = get4b(pos);

    binary &= 0xFF000000;
    binary |= (value % 0x1000000);
    put4b(pos, binary);
}

uint32_t CodeGenerator::getLow24(int pos) { return (get4b(pos) % 0x1000000); }

// If 2 is a square root of x, returns number of shifts required to reach x,
// or returns zero otherwise.
unsigned CodeGenerator::shiftBy(int x)
{
    int a, n;

    if ((x % 2) == 1)
        return 0;

    a = x / 2;
    n = 2;

    while ((a % 2) != 1 && a != 2)
    {
        a /= 2;
        n++;
    }

    if (a == 2 && n < 32)
        return n;

    return 0;
}

void CodeGenerator::putAsmStr(const char *str)
{
    asmBuf.push_back(std::string(str));
}

void CodeGenerator::setStaticDataSize(int size)
{
    objheader.staticDataSize = size;
}

void CodeGenerator::setMainPC() { objheader.mainPC = PC; }

bool CodeGenerator::addSymbol(const char *name, int offset)
{
    for (unsigned i = 0; i < symbols.size(); i++)
    {
        if (strcmp(symbols[i].name, name) == 0)
        {
            // Symbol already exist, check if was a forward declaration.
            if (symbols[i].offset == 0xFFFFFFFF)
            {
                // Define the address of forward declared function.
                symbols[i].offset = offset;

                elfObject.addSymbol(name, offset);
                return true;
            }

            // Not forward declaration of function, return false.
            return false;
        }
    }

    struct symbol_t symbol;

    strcpy(symbol.name, name);
    symbol.offset = offset;
    symbols.push_back(symbol);

    /*if (elfObject.symbolExists(name))
    {
        if (elfObject.getSymbolValue(name) == 0xffffffff)
        {
        elfObject.setSymbolValue(name, offset);
        return true;
        }
        return false;
    }

    elfObject.addSymbol(name, offset);*/

    return true;
}

void CodeGenerator::addRelocation(unsigned type,
                                      unsigned offset,
                                      const char *value)
{
    Elf32_Rel reloc;

    // Relocation type + symbol's index in the symbol table
    reloc.r_info = ELF32_R_INFO(0, type);

    // Position of this relocation
    reloc.r_offset = offset;

    // FIXME: This is just a temporary solution, should be removed in future,
    // because symbol table index of the symbol should be used instead.

#ifndef GNU_ABI
    strcpy(reloc.value, value);
#endif
    relocs.push_back(reloc);

    elfObject.addRelocation(type, offset, value);
}

unsigned CodeGenerator::addStringToReadOnlyData(const char *value)
{
    unsigned pos;
    const char *ch;

    if (rodata_ptr == 0)
    {
        // TODO: Add symbol '.rodata'
    }

    pos = rodata_ptr;
    ch = value;
    while (*ch != '\0')
    {
        rodata[rodata_ptr] = *ch;
        rodata_ptr++;
        ch++;
    }

    rodata[rodata_ptr++] = '\0';
    if ((rodata_ptr % 4) != 0)
        rodata_ptr += (rodata_ptr % 4);

    // char *tmp = (char *) rodata+pos;
    // printf("--- rodata = %s, pos = %d\n", tmp, pos);

    return pos;
}

// TODO: Use C++ functions from <fstream>.
// Store the code buffer into filename.out file
void CodeGenerator::write(const char *output)
{
    int n, totalNumberOfBytes;
    FILE *fp;

    if (asmOutput)
    {
        fp = fopen(output, "w");
        if (fp == NULL)
        {
            std::cerr << "error: cannot create output filename \"" << output
                      << "\"" << std::endl;
            exit(1);
        }

        fprintf(fp, "\t.text\n");

        for (unsigned i = 0; i < asmBuf.size(); i++)
            fprintf(fp, "%s\n", asmBuf[i].c_str());

        std::cout << "Output: \"" << output << "\"" << std::endl;
        fclose(fp);
    }
    else
    {
#ifdef GNU_ABI
        elfObject.writeToFile(fp);
#else
        fp = fopen(output, "wb");
        if (fp == NULL)
        {
            std::cerr << "error: cannot create output filename \"" << output
                      << "\"\n" << std::endl;
            exit(1);
        }

        resolveFixups();

        totalNumberOfBytes = 0;

        objheader.hflags = 0x0; // Mark the file as an object.
        objheader.codeSize = PC;
        objheader.readOnlyDataSize = rodata_ptr;
        objheader.relocsc = relocs.size();
        objheader.symbolsc = symbols.size();

        // Write the number of bytes
        n = fwrite(&objheader, sizeof(struct objheader_t), 1, fp);
        if (!n)
        {
            std::cerr << "fatal error: cannot write code size" << std::endl;
            exit(1);
        }
        else
        {
            totalNumberOfBytes += n;
        }

        // Read only data
        if (objheader.readOnlyDataSize > 0)
        {
            n = fwrite(rodata, sizeof(char), objheader.readOnlyDataSize, fp);
            if (!n)
            {
                std::cerr << "fatal error: cannot write read only data"
                          << std::endl;
                exit(1);
            }
            else
            {
                totalNumberOfBytes += n;
            }
        }

        // Static data
        if (objheader.staticDataSize > 0)
        {
            n = fwrite(data, sizeof(unsigned), objheader.staticDataSize, fp);
            if (!n)
            {
                std::cerr << "fatal error: cannot write static data"
                          << std::endl;
                exit(1);
            }
            else
            {
                totalNumberOfBytes += n;
            }
        }

        // Symbol table
        if (symbols.size() > 0)
        {
            n = fwrite(&symbols[0], sizeof(struct symbol_t), symbols.size(), fp);
            if (!n)
            {
                std::cerr << "fatal error: cannot write symbol table"
                          << std::endl;
                exit(1);
            }
            else
            {
                totalNumberOfBytes += n;
            }
        }

        // Relocations
        if (relocs.size() > 0)
        {
            n = fwrite(&relocs[0], sizeof(Elf32_Rel), relocs.size(), fp);
            if (!n)
            {
                std::cerr << "fatal error: cannot write relocations\n"
                          << std::endl;
                exit(1);
            }
            else
            {
                totalNumberOfBytes += n;
            }
        }

        // Write the code buffer
        n = fwrite(&buf[0], sizeof(char), objheader.codeSize, fp);
        if (!n)
        {
            std::cerr << "fatal error: cannot write output" << std::endl;
            exit(1);
        }
        else
        {
            totalNumberOfBytes += n;
            std::cout << "Output: \"" << output << "\"" << std::endl;
            std::cout << "Total number of bytes: " << totalNumberOfBytes
                      << std::endl;
        }
#endif
        // FIXME: The fclose produces a memory error.
        // Check links:
        //     http://stackoverflow.com/questions/29870357/writing-struct-writebuf-points-to-uninitialised-bytes
        //     http://stackoverflow.com/questions/12419309/valgrind-error-when-writing-struct-to-file-with-fwrite-syscall-param-writebuf
        //     http://stackoverflow.com/questions/15779735/file-reading-and-writing-creates-valgrind-error

        fclose(fp);
    }
}

// PICO Linker, v0.2
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "../include/ARM.h"
#include "../include/ELF.h"
#include "../include/PiccoObjectFormat.h"
#include "../include/common.h"

#define MAX_FILENAME 256
#define MAX_DATA     1000

struct Object
{
    char filename[MAX_FILENAME];

    struct objheader_t header;
    struct symbol_t *symbols = nullptr;
    uint8_t *rodata = nullptr;
    uint32_t *data = nullptr;
    Elf32_Rel *relocs = nullptr;
    uint8_t *code = nullptr; // The code buffer. TODO: Convert it to a vector.
};

class Linker
{
protected:
    bool isLittleEndian;
    std::vector<Object *> obj;
    uint32_t *exebuf = nullptr;
    unsigned exebufSize;
    unsigned rodataSize;

    uint32_t data[MAX_DATA];
    unsigned dataSize;
    unsigned staticDataSize_b;

    uint8_t get1b(uint8_t *buf, int pc) { return buf[pc]; }

    uint16_t get2b(uint8_t *buf, int pc)
    {
        if (isLittleEndian)
            return (get1b(buf, pc) | (get1b(buf, pc + 1) << 8));

        return ((get1b(buf, pc) << 8) | get1b(buf, pc + 1));
    }

    uint32_t get4b(uint8_t *buf, int pc)
    {
        if (isLittleEndian)
            return (get2b(buf, pc) | (get2b(buf, pc + 2) << 16));

        return ((get2b(buf, pc) << 16) | get2b(buf, pc + 2));
    }

    void put1b(uint8_t *buf, int pos, uint8_t x) { buf[pos] = x; }

    void put2b(uint8_t *buf, int pos, uint16_t x)
    {
        if (isLittleEndian)
        {
            put1b(buf, pos, x);
            put1b(buf, pos + 1, x >> 8);
        }
        else
        {
            put1b(buf, pos, x >> 8);
            put1b(buf, pos + 1, x);
        }
    }

    void put4b(uint8_t *buf, int pos, uint32_t x)
    {
        if (isLittleEndian)
        {
            put2b(buf, pos, x);
            put2b(buf, pos + 2, x >> 16);
        }
        else
        {
            put2b(buf, pos, x >> 16);
            put2b(buf, pos + 2, x);
        }
    }

public:
    Linker(bool LE)
        : isLittleEndian(LE), exebufSize(0), rodataSize(0), dataSize(0),
          staticDataSize_b(0) {}

    virtual ~Linker();

    void link(int argc, char **argv, const char *output);

    void loadObjects(int argc, char **argv);
    void combineObjects();
    virtual void resolveRelocations() {}
    void writeExecutable(const char *output);
};

Linker::~Linker()
{
    if (exebuf)
        delete exebuf;

    for (unsigned i = 0; i < obj.size(); i++)
    {
        // Release the object data
        if (obj[i]->rodata)
            delete obj[i]->rodata;
        if (obj[i]->data)
            delete obj[i]->data;
        if (obj[i]->symbols)
            delete obj[i]->symbols;
        if (obj[i]->relocs)
            delete obj[i]->relocs;
        if (obj[i]->code)
            delete obj[i]->code;
        // Release the object
        delete obj[i];
    }
}

void Linker::link(int argc, char **argv, const char *output)
{
    loadObjects(argc, argv);
    combineObjects();
    resolveRelocations();
    writeExecutable(output);
}

void Linker::loadObjects(int argc, char **argv)
{
    Object *tmp = nullptr;

    for (int i = 0; i < argc - 1; i++)
    {
        char *filename = argv[i + 1];
        FILE *fp = fopen(filename, "rb");

        if (fp == nullptr)
        {
            printf("Filename read error.\n");
            exit(1);
        }

        tmp = new Object();

        strcpy(tmp->filename, filename);

        fread(&(tmp->header), sizeof(struct objheader_t), 1, fp);

        if (tmp->header.readOnlyDataSize > 0)
        {
            tmp->rodata = new uint8_t[tmp->header.readOnlyDataSize];
            fread(tmp->rodata, sizeof(uint8_t), tmp->header.readOnlyDataSize,
                  fp);
        }

        // TODO: Write data to final executable
        if (tmp->header.staticDataSize > 0)
        {
            tmp->data = new uint32_t[tmp->header.staticDataSize];
            fread(tmp->data, sizeof(uint32_t), tmp->header.staticDataSize, fp);
        }

        if (tmp->header.symbolsc > 0)
        {
            tmp->symbols = new symbol_t[tmp->header.symbolsc];
            fread(tmp->symbols, sizeof(struct symbol_t), tmp->header.symbolsc,
                  fp);
        }

        if (tmp->header.relocsc > 0)
        {
            tmp->relocs = new Elf32_Rel[tmp->header.relocsc];
            fread(tmp->relocs, sizeof(Elf32_Rel), tmp->header.relocsc, fp);
        }

        tmp->code = new uint8_t[tmp->header.codeSize];
        fread(tmp->code, sizeof(uint8_t), tmp->header.codeSize, fp);

        fclose(fp);

        printf("\nFilename:   %s\n\n", tmp->filename);
        printf("  Code size:                %d\n", tmp->header.codeSize);
        printf("  Static data size:         %d\n", tmp->header.staticDataSize);
        printf("  Read only data size:      %d\n",
               tmp->header.readOnlyDataSize);
        printf("  Main PC:                  %d\n", tmp->header.mainPC);
        printf("  Symbol table size:        %d\n", tmp->header.symbolsc * 4);
        printf("  Relocation section size:  %d\n", tmp->header.relocsc * 4);
        printf("\n");

        obj.push_back(tmp);
    }
}

void Linker::combineObjects()
{
    Object *tmp = nullptr;

    unsigned maxExebufSize = 0;
    for (unsigned i = 0; i < obj.size(); i++)
    {
        maxExebufSize += obj[i]->header.codeSize;
        maxExebufSize += obj[i]->header.readOnlyDataSize;
        // Static data size is in bytes
        maxExebufSize += (obj[i]->header.staticDataSize / 4);
    }

    if (maxExebufSize == 0)
        return;

    exebuf = new uint32_t[maxExebufSize];

    // Put all the code together.
    for (int i = obj.size() - 1; i >= 0; i--)
    {
        tmp = obj[i];

        // Copy the read only data.
        // TODO: So far this works only for single object file.
        // TODO: rodata must be loaded as a four bytes per exebuf entry.

        exebuf[exebufSize] = 0;
        for (unsigned n = 0; n < tmp->header.readOnlyDataSize; n++)
        {
            if (n > 0 && (n % 4) == 0)
            {
                exebufSize++;
                exebuf[exebufSize] = 0;
            }
            exebuf[exebufSize] =
                exebuf[exebufSize] | (tmp->rodata[n] << ((n % 4) * 8));
        }
        rodataSize += tmp->header.readOnlyDataSize;

        // Align the exebufSize
        if ((exebufSize % 4) != 0)
        {
            exebufSize += (exebufSize % 4);
        }

        // Recalculate offsets of a symbols in the symbol table.
        for (unsigned n = 0; n < tmp->header.symbolsc; n++)
            tmp->symbols[n].offset += (exebufSize * 4);

        // Recalculate offsets of a relocations.
        for (unsigned n = 0; n < tmp->header.relocsc; n++)
            tmp->relocs[n].r_offset += (exebufSize * 4);

        tmp->header.mainPC += (exebufSize * 4);

        staticDataSize_b += tmp->header.staticDataSize;

        // Copy the code.
        for (unsigned n = 0; n < tmp->header.codeSize; n += 4)
        {
            // Get the code from specified endianness.
            exebuf[exebufSize] = get4b(tmp->code, n);
            exebufSize++;
        }

        // Copy the static data.
        unsigned staticDataSizeInWords = tmp->header.staticDataSize / 4;
        for (unsigned n = 0; n < staticDataSizeInWords; n++)
        {
            data[dataSize] = tmp->data[n];
            dataSize++;
        }
    }

    unsigned k = 0;
    // Add static data to final executable
    for (unsigned n = 0; n < (staticDataSize_b / 4); n++)
    {
        exebuf[exebufSize] = data[k];
        exebufSize++;
        k++;
    }
}

void Linker::writeExecutable(const char *output)
{
    int n, totalNumberOfBytes;
    FILE *fp;
    Object *last = obj[0];

    fp = fopen(output, "wb");
    if (fp == nullptr)
    {
        printf("error: cannot create output filename \"%s\"\n", output);
        exit(1);
    }

    totalNumberOfBytes = 0;
    last->header.hflags = 0x1;
    // TODO: Implement this
    last->header.readOnlyDataSize = rodataSize;
    last->header.codeSize = (exebufSize * 4) - staticDataSize_b;

    // Write the header.
    n = fwrite(&(last->header), sizeof(struct objheader_t), 1, fp);
    if (!n)
    {
        printf("fatal error: cannot write code size\n");
        exit(1);
    }
    else
    {
        totalNumberOfBytes += n;
    }

    // FIXME: rodata should be part of exebuf!!!
    // Otherwise it cannot be used:
    // addiu, $v0, $v0, ???
    // if (rodataSize > 0)
    //  fwrite(&rodata, sizeof(char), rodataSize, fp);

    // This step returns bytes to specified endianness.
    uint8_t *tmpExebuf = new uint8_t[exebufSize * 4];

    for (unsigned i = 0; i < exebufSize; i++)
        put4b(tmpExebuf, (i*4), exebuf[i]);

    // TODO: Save in correct endianess.
    // Write the code buffer
    n = fwrite(tmpExebuf, sizeof(char), (exebufSize * 4), fp);

    delete tmpExebuf;

    if (!n)
    {
        printf("fatal error: cannot write output\n");
        exit(1);
    }
    else
    {
        totalNumberOfBytes += n;
        printf("Output: \"%s\"\n", output);
        printf("Total number of bytes: %d\n", totalNumberOfBytes);
    }
}

//
//  ARM
//

class ARMLinker : public Linker
{
public:
    ARMLinker() : Linker(true) {}
    void resolveRelocations();
};

void ARMLinker::resolveRelocations()
{
    Object *tmp = nullptr;

    for (unsigned i = 0; i < obj.size(); i++)
    {
        tmp = obj[i];

        for (unsigned n = 0; n < tmp->header.relocsc; n++)
        {
            Elf32_Rel rel;
            unsigned rel_type;

            rel = tmp->relocs[n];
            rel_type = ELF32_R_TYPE(rel.r_info);

            switch (rel_type)
            {
                case R_ARM_CALL:
                case R_ARM_JUMP24:
                {
                    unsigned target = 0;
                    bool found = false;

                    // Find the target symbol and its address.
                    for (unsigned k = 0; k < obj.size(); k++)
                    {
                        for (unsigned m = 0; m < obj[k]->header.symbolsc; m++)
                        {
                            struct symbol_t sym = obj[k]->symbols[m];

#ifndef GNU_ABI
                            if (strcmp(rel.value, sym.name) == 0)
                            {
                                if (sym.offset != (unsigned)-1)
                                {
                                    target = sym.offset;
                                    found = true;
                                    break;
                                }
                            }
#endif
                        }
                    }
                    if (found)
                    {
                        unsigned offset = rel.r_offset / 4;
                        unsigned binary = exebuf[offset];

                        binary &= 0xFC000000; // binary{26-0} = 0x0

                        // binary{26-0} = target
                        binary |= (target >> 2) % 0x4000000;

                        exebuf[offset] = binary;
                    }
                }
                break;

                default:
                    break;
            }
        }
    }
}

// Driver

int main(int argc, char **argv)
{
    char output[256], target[256];
    // char *input;
    bool outputOk, printHelp, printVersion;
    int n;
    Linker *lnk;

    if (argc <= 1)
    {
        // print_info();
        // print_usage(argv[0]);
        printf("Try -h option for more info.\n");
        exit(1);
    }

    strcpy(target, "arm");
    // input = nullptr;
    printHelp = false;
    outputOk = false;
    n = 1;

    while (n < argc)
    {
        if (strcmp(argv[n], "-o") == 0 || strcmp(argv[n], "--output") == 0)
        {
            strcpy(output, argv[++n]);
            outputOk = true;
            argc--;
        }
        else if (strcmp(argv[n], "--target") == 0)
        {
            strcpy(target, argv[++n]);
            argc -= 2;
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
        /*else
        {
        input = argv[n];
        }*/
        n++;
    }

    if (strcmp(target, "arm") == 0)
        lnk = new ARMLinker();

    lnk->link(argc, argv, outputOk ? output : "output.out");
    delete lnk;

    return 0;
}

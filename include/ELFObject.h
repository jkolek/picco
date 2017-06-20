// ELF object support - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef ELF_OBJECT_H
#define ELF_OBJECT_H

#include "ELF.h"
#include "common.h"
#include <cstdio>
#include <cstring>
#include <vector>

#define NULL_SHDR shdrtab[0]
#define SHSTRTAB_SHDR shdrtab[1]
#define STRTAB_SHDR shdrtab[2]
#define SYMTAB_SHDR shdrtab[3]
#define RELOCS_SHDR shdrtab[4]
#define TEXT_SHDR shdrtab[5]

#define SHDRTAB_NUM 6

class ELFSection
{
protected:
    Elf32_Shdr header;
    unsigned index;

public:
    ELFSection()
    {
        header.sh_flags = 0;
        header.sh_addr = 0;
        header.sh_size = 0;
        header.sh_link = 0;
        header.sh_info = 0;
        header.sh_addralign = 0;
        header.sh_entsize = 0;
        index = 0;
    }
    virtual ~ELFSection() {}

    void setName(Elf32_Word name) { header.sh_name = name; }
    void setType(Elf32_Word type) { header.sh_type = type; }
    void setFlags(Elf32_Word flags) { header.sh_flags = flags; }
    void setAddr(Elf32_Addr addr) { header.sh_addr = addr; }
    void setOffset(Elf32_Off offset) { header.sh_offset = offset; }
    void setSize(Elf32_Word size) { header.sh_size = size; }
    void setLink(Elf32_Word link) { header.sh_link = 0x0; }
    void setInfo(Elf32_Word info) { header.sh_info = 0x0; }
    void setAddrAlign(Elf32_Word align) { header.sh_addralign = align; }
    void setEntSize(Elf32_Word entSize) { header.sh_entsize = entSize; }

    Elf32_Word getName() { return header.sh_name; }
    Elf32_Word getType() { return header.sh_type; }
    Elf32_Word getFlags() { return header.sh_flags; }
    Elf32_Addr getAddr() { return header.sh_addr; }
    Elf32_Off getOffset() { return header.sh_offset; }
    Elf32_Word getSize() { return header.sh_size; }
    Elf32_Word getLink() { return header.sh_link; }
    Elf32_Word getInfo() { return header.sh_info; }
    Elf32_Word getAddrAlign() { return header.sh_addralign; }
    Elf32_Word getEntSize() { return header.sh_entsize; }

    const Elf32_Shdr *getHeaderData() { return &header; }
    unsigned getIndex() { return index; }
    void setIndex(unsigned i) { index = i; }
    virtual void update() {}
};

class StringTableSection : public ELFSection
{
    char stringTable[1000];
    unsigned stringTableIdx;

public:
    StringTableSection();
    unsigned appendString(const char *s);

    char *getData() { return stringTable; }
    unsigned getDataSize() { return stringTableIdx; }
    void update() { header.sh_size = stringTableIdx; }
};

class SymbolTableSection : public ELFSection
{
    Elf32_Sym symbols[1000];
    unsigned symbolsIdx;

    StringTableSection *strtab;

public:
    SymbolTableSection(StringTableSection *StrTab);
    void addSymbol(const char *name, unsigned offset);
    bool symbolExists(const char *name);
    bool setSymbolValue(const char *name, unsigned value);
    unsigned getSymbolValue(const char *name);

    Elf32_Sym *getData() { return symbols; }
    unsigned getDataSize() { return symbolsIdx * sizeof(Elf32_Sym); }

    void update() { header.sh_size = symbolsIdx * sizeof(Elf32_Sym); }
};

class RelocationSection : public ELFSection
{
    Elf32_Rel relocs[1000];
    unsigned relocsIdx;

    SymbolTableSection *symtab;

public:
    RelocationSection(SymbolTableSection *SymTab);
    void addRelocation(unsigned type, unsigned offset, const char *value);

    Elf32_Rel *getData() { return relocs; }
    unsigned getDataSize() { return relocsIdx * sizeof(Elf32_Rel); }

    void update() { header.sh_size = relocsIdx * sizeof(Elf32_Rel); }
};

class TextSection : public ELFSection
{
    unsigned dataSize;
    uint8_t *data;

public:
    TextSection()
    {
        header.sh_type = SHT_PROGBITS;
        header.sh_flags = 0;
        header.sh_addr = 0;
        header.sh_link = 0;
        header.sh_info = 0;
        header.sh_addralign = 4;
        header.sh_entsize = 0;
        dataSize = 0;
        data = nullptr;
    }

    void setData(uint8_t *Data) { data = Data; }
    void setDataSize(unsigned DataSize) { dataSize = DataSize; }

    uint8_t *getData() { return data; }
    unsigned getDataSize() { return dataSize; }

    void update() { header.sh_size = dataSize; }
};

class SectionHeader
{
    // Section header
    Elf32_Shdr header;

public:
    SectionHeader(Elf32_Word type) { header.sh_type = type; }
    ~SectionHeader() {}

    void setName(Elf32_Word name) { header.sh_name = name; }
    void setType(Elf32_Word type) { header.sh_type = type; }
    void setFlags(Elf32_Word flags) { header.sh_flags = flags; }
    void setAddr(Elf32_Addr addr) { header.sh_addr = addr; }
    void setOffset(Elf32_Off offset) { header.sh_offset = offset; }
    void setSize(Elf32_Word size) { header.sh_size = size; }
    void setLink(Elf32_Word link) { header.sh_link = 0x0; }
    void setInfo(Elf32_Word info) { header.sh_info = 0x0; }
    void setAddrAlign(Elf32_Word align) { header.sh_addralign = align; }
    void setEntSize(Elf32_Word entSize) { header.sh_entsize = entSize; }

    Elf32_Word getName() { return header.sh_name; }
    Elf32_Word getType() { return header.sh_type; }
    Elf32_Word getFlags() { return header.sh_flags; }
    Elf32_Addr getAddr() { return header.sh_addr; }
    Elf32_Off getOffset() { return header.sh_offset; }
    Elf32_Word getSize() { return header.sh_size; }
    Elf32_Word getLink() { return header.sh_link; }
    Elf32_Word getInfo() { return header.sh_info; }
    Elf32_Word getAddrAlign() { return header.sh_addralign; }
    Elf32_Word getEntSize() { return header.sh_entsize; }

    Elf32_Shdr &getData() { return header; }
};

class ELFObject
{
    // ELF header
    Elf32_Ehdr ehdr;

    ELFSection *nullSection;
    StringTableSection *shStrtabSection;
    StringTableSection *strtabSection;
    SymbolTableSection *symtabSection;
    RelocationSection *relocSection;
    TextSection *txtSection;

    std::vector<ELFSection *> sections;

public:
    void setEhdrType(Elf32_Half type) { ehdr.e_type = type; }
    void setEhdrMachine(Elf32_Half machine) { ehdr.e_machine = machine; }
    void setEhdrVersion(Elf32_Word version) { ehdr.e_version = version; }
    void setEhdrEntry(Elf32_Addr entry) { ehdr.e_entry = entry; }
    void setEhdrPhoff(Elf32_Off phoff) { ehdr.e_phoff = phoff; }
    void setEhdrShoff(Elf32_Off shoff) { ehdr.e_shoff = shoff; }
    void setEhdrFlags(Elf32_Word flags) { ehdr.e_flags = flags; }
    void setEhdrEhsize(Elf32_Half ehsize) { ehdr.e_ehsize = ehsize; }
    void setEhdrPhentsize(Elf32_Half phentsize)
    {
        ehdr.e_phentsize = phentsize;
    }
    void setEhdrPhnum(Elf32_Half phnum) { ehdr.e_phnum = phnum; }
    void setEhdrShentsize(Elf32_Half shentsize)
    {
        ehdr.e_shentsize = shentsize;
    }
    void setEhdrShnum(Elf32_Half shnum) { ehdr.e_shnum = shnum; }
    void setEhdrShstrndx(Elf32_Half shstrndx) { ehdr.e_shstrndx = shstrndx; }

    void addSymbol(const char *name, unsigned offset)
    {
        symtabSection->addSymbol(name, offset);
    }

    void addRelocation(unsigned type, unsigned offset, const char *value)
    {
        relocSection->addRelocation(type, offset, value);
    }

    void setTextSection(uint8_t *txtSec) { txtSection->setData(txtSec); }

    void setTextSectionSize(unsigned size) { txtSection->setDataSize(size); }

    void setSectionHeaderTable();
    void writeToFile(FILE *fp);
    void readFromFile(FILE *fp);

    ELFObject();
    ~ELFObject();
};

#endif

// ELF object support - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include "../include/ELFObject.h"
#include "../include/ELF.h"
#include <cstring>
#include <vector>

//
//  String table section
//

StringTableSection::StringTableSection()
{
    header.sh_type = SHT_STRTAB;
    header.sh_flags = 0;
    header.sh_addr = 0;
    header.sh_size = 0;
    header.sh_link = 0;
    header.sh_info = 0;
    header.sh_addralign = 1;
    header.sh_entsize = 0;

    stringTable[0] = '\0';
    stringTableIdx = 1;
}

unsigned StringTableSection::appendString(const char *s)
{
    unsigned index = stringTableIdx;
    unsigned n = 0;

    do
    {
        stringTable[stringTableIdx] = s[n];
        stringTableIdx++;
        n++;
    } while (s[n] != '\0');
    stringTable[stringTableIdx++] = '\0';

    return index;
}

//
//  Symbol table section
//

SymbolTableSection::SymbolTableSection(StringTableSection *StrTab)
    : strtab(StrTab)
{
    header.sh_type = SHT_SYMTAB;
    header.sh_flags = 0x0;
    header.sh_addr = 0x0;
    header.sh_link = strtab->getIndex(); // Link to the .strtab
    header.sh_info = 0x0;
    header.sh_addralign = 2;
    header.sh_entsize = sizeof(Elf32_Sym);

    symbolsIdx = 0;
}

void SymbolTableSection::addSymbol(const char *name, unsigned offset)
{
    unsigned nameIdx = strtab->appendString(name);

    symbols[symbolsIdx].st_name = nameIdx;
    symbols[symbolsIdx].st_value = offset;
    // FIXME: This should not be fixed to STB_GLOBAL and STT_FUNC.
    symbols[symbolsIdx].st_info = ELF32_ST_INFO(STB_GLOBAL, STT_FUNC);
    symbols[symbolsIdx].st_other = 0;
    symbols[symbolsIdx].st_shndx = 5; // Text section index
    // symbols[symbolsIdx].st_size = 32; // Text section size
    symbolsIdx++;
}

bool SymbolTableSection::symbolExists(const char *name)
{
    char *stringTable = strtab->getData();

    for (unsigned i = 0; i < symbolsIdx; i++)
    {
        char *sym = stringTable + symbols[i].st_name;
        if (strcmp(sym, name) == 0)
            return true;
    }
    return false;
}

bool SymbolTableSection::setSymbolValue(const char *name, unsigned value)
{
    char *stringTable = strtab->getData();

    for (unsigned i = 0; i < symbolsIdx; i++)
    {
        char *sym = stringTable + symbols[i].st_name;
        if (strcmp(sym, name) == 0)
        {
            symbols[i].st_value = value;
            return true;
        }
    }
    return false;
}

unsigned SymbolTableSection::getSymbolValue(const char *name)
{
    char *stringTable = strtab->getData();

    for (unsigned i = 0; i < symbolsIdx; i++)
    {
        char *sym = stringTable + symbols[i].st_name;
        if (strcmp(sym, name) == 0)
            return symbols[i].st_value;
    }
    return -1;
}

//
//  Relocation section
//

RelocationSection::RelocationSection(SymbolTableSection *SymTab)
    : symtab(SymTab)
{
    header.sh_type = SHT_REL;
    header.sh_flags = 0x0;
    header.sh_addr = 0x0;
    header.sh_link = symtab->getIndex(); // Link to the .symtab
    header.sh_info = 0x0;
    header.sh_addralign = 1;
    header.sh_entsize = sizeof(Elf32_Rel);

    relocsIdx = 0;
}

void RelocationSection::addRelocation(unsigned type,
                                      unsigned offset,
                                      const char *value)
{
    // Relocation type + symbol's index in the symbol table
    relocs[relocsIdx].r_info =
        ELF32_R_INFO(symtab->getSymbolValue(value), type);

    // Position of this relocation
    relocs[relocsIdx].r_offset = offset;

    // FIXME: This is just a temporary solution, should be removed in future,
    // because symbol table index of the symbol should be used instead.
    // strcpy(relocs[objheader.relocsc].value, value);

    relocsIdx++;
}

//
//  ELF object file
//

void ELFObject::setSectionHeaderTable()
{
    Elf32_Off offset;

    /*for (unsigned i = 0; i < stringTableIdx; i++)
    {
      if (stringTable[i] == '\0')
        printf("-");
      else
        printf("%c", stringTable[i]);
    }*/

    shStrtabSection->setName(shStrtabSection->appendString(".shstrtab"));
    strtabSection->setName(shStrtabSection->appendString(".strtab"));
    symtabSection->setName(shStrtabSection->appendString(".symtab"));
    relocSection->setName(shStrtabSection->appendString(".relocs"));
    txtSection->setName(shStrtabSection->appendString(".text"));

    // calculateOffsets();

    for (unsigned i = 0; i < sections.size(); i++)
        sections[i]->update();

    offset = sizeof(Elf32_Ehdr) + (sections.size() * sizeof(Elf32_Shdr));
    for (unsigned i = 0; i < sections.size(); i++)
    {
        sections[i]->setOffset(offset);
        offset += sections[i]->getSize();
    }
}

void ELFObject::writeToFile(FILE *fp)
{
    setSectionHeaderTable();

    fwrite(&ehdr, sizeof(Elf32_Ehdr), 1, fp);

    // Write section headers
    for (unsigned i = 0; i < sections.size(); i++)
        fwrite(sections[i]->getHeaderData(), sizeof(Elf32_Shdr), 1, fp);

    fwrite(shStrtabSection->getData(), sizeof(char),
           shStrtabSection->getDataSize(), fp);
    fwrite(strtabSection->getData(), sizeof(char), strtabSection->getDataSize(),
           fp);
    fwrite(symtabSection->getData(), symtabSection->getDataSize(), 1, fp);
    fwrite(relocSection->getData(), relocSection->getDataSize(), 1, fp);
    fwrite(txtSection->getData(), txtSection->getDataSize(), 1, fp);
}

void ELFObject::readFromFile(FILE *fp)
{
    /*  setSectionHeaderTable();

      fread(&ehdr, sizeof(Elf32_Ehdr), 1, fp);

      // Read section headers
      for (unsigned i = 0; i < sections.size(); i++)
      {
        Elf32_Shdr shdr;
        fread(&shdr, sizeof(Elf32_Shdr), 1, fp);
        sections[i]->setHeaderData(shdr);
      }

      fwrite(shStrtabSection->getData(), sizeof(char),
             shStrtabSection->getDataSize(), fp);
      fwrite(strtabSection->getData(), sizeof(char),
             strtabSection->getDataSize(), fp);
      fwrite(symtabSection->getData(), symtabSection->getDataSize(), 1, fp);
      fwrite(relocSection->getData(), relocSection->getDataSize(), 1, fp);
      fwrite(txtSection->getData(), txtSection->getDataSize(), 1, fp);*/
}

ELFObject::ELFObject()
{
    unsigned index = 0;

    // File identification
    ehdr.e_ident[EI_MAG0] = 0x7f;
    ehdr.e_ident[EI_MAG1] = 'E';
    ehdr.e_ident[EI_MAG2] = 'L';
    ehdr.e_ident[EI_MAG3] = 'F';
    ehdr.e_ident[EI_CLASS] = ELFCLASS32;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    // ehdr.e_ident[EI_DATA] = ELFDATA2MSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    ehdr.e_ident[EI_PAD] = 8;

    nullSection = new ELFSection();
    nullSection->setType(SHT_NULL);
    nullSection->setIndex(index++);
    sections.push_back(nullSection);

    shStrtabSection = new StringTableSection();
    shStrtabSection->setIndex(index++);
    sections.push_back(shStrtabSection);

    strtabSection = new StringTableSection();
    strtabSection->setIndex(index++);
    sections.push_back(strtabSection);

    symtabSection = new SymbolTableSection(strtabSection);
    symtabSection->setIndex(index++);
    sections.push_back(symtabSection);

    relocSection = new RelocationSection(symtabSection);
    relocSection->setIndex(index++);
    sections.push_back(relocSection);

    txtSection = new TextSection();
    txtSection->setIndex(index++);
    sections.push_back(txtSection);
}

ELFObject::~ELFObject()
{
    delete nullSection;
    delete shStrtabSection;
    delete strtabSection;
    delete symtabSection;
    delete relocSection;
    delete txtSection;
}

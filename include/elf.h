#ifndef ELF_H
#define ELF_H

#include <stddef.h>
#include <stdint.h>

// ponytail: own ELF64 defs instead of <elf.h> -- Darwin has none, and a
// system include of "elf.h" resolves back to this file under -Iinclude.
// 64-bit fields are `unsigned long` (not uint64_t) so the existing "%lx"
// printfs are correct on LP64; the asserts below reject non-LP64 builds.

#define EI_NIDENT 16

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASS64 2
#define ELFDATA2LSB 1

#define EM_X86_64 62
#define SHN_UNDEF 0

typedef struct {
    uint8_t e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    unsigned long e_entry;
    unsigned long e_phoff;
    unsigned long e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    unsigned long p_offset;
    unsigned long p_vaddr;
    unsigned long p_paddr;
    unsigned long p_filesz;
    unsigned long p_memsz;
    unsigned long p_align;
} Elf64_Phdr;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    unsigned long sh_flags;
    unsigned long sh_addr;
    unsigned long sh_offset;
    unsigned long sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    unsigned long sh_addralign;
    unsigned long sh_entsize;
} Elf64_Shdr;

_Static_assert(sizeof(Elf64_Ehdr) == 64, "Elf64_Ehdr layout");
_Static_assert(sizeof(Elf64_Phdr) == 56, "Elf64_Phdr layout");
_Static_assert(sizeof(Elf64_Shdr) == 64, "Elf64_Shdr layout");

typedef struct {
    uint8_t *data;
    size_t size;

    Elf64_Ehdr *header;

    Elf64_Phdr *program_headers;
    Elf64_Shdr *section_headers;

    char *section_names;
} ElfFile;

int elf_load(const char *path, ElfFile *elf);
void elf_free(ElfFile *elf);

void elf_print_header(const ElfFile *elf);
void elf_print_sections(const ElfFile *elf);
void elf_print_program_headers(const ElfFile *elf);

Elf64_Shdr *elf_find_section(
    const ElfFile *elf,
    const char *name
);

#endif //ELF_H

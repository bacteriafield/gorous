#include "elf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int elf_validate(const Elf64_Ehdr *header) {
    if (header->e_ident[EI_MAG0] != ELFMAG0 ||
        header->e_ident[EI_MAG1] != ELFMAG1 ||
        header->e_ident[EI_MAG2] != ELFMAG2 ||
        header->e_ident[EI_MAG3] != ELFMAG3) {

        fprintf(stderr, "Not an ELF file\n");
        return 0;
    }
    
    if (header->e_ident[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "Only ELF64 is supported\n");
        return 0;
    }

    if (header->e_ident[EI_DATA] != ELFDATA2LSB) {
        fprintf(stderr, "Only little-endian ELF is supported\n");
        return 0;
    }

    if (header->e_machine != EM_X86_64) {
        fprintf(stderr, "Only x86-64 is supported\n");
        return 0;
    }

    return 1;
}

int elf_load(const char *path, ElfFile *elf) {
    FILE *file = fopen(path, "rb");
    
    if (!file) {
        perror("fopen");
        return -1;
    }
    
    memset(elf, 0, sizeof(*elf));
    
    //tell file size
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }
    
    long file_size = ftell(file);
    
    if (file_size < 0) {
        fclose(file);
        return -1;
    }
    
    rewind(file);
    
    // load file at memory
    elf->size = (size_t)file_size;
    elf->data = malloc(elf->size);
    
    if (!elf->data) {
        fclose(file);
        return -1;
    }
    
    size_t read = fread(elf->data, 1, elf->size, file);
    
    fclose(file);
    
    if (read != elf->size) {
        fprintf(stderr, "Failed to read ELF file\n");
        elf_free(elf);
        return -1;
    }
    
    // elf header stay on beginning
    if (elf->size < sizeof(Elf64_Ehdr)) {
        fprintf(stderr, "File is too small to be an ELF\n");
        elf_free(elf);
        return -1;
    }
    
    elf->header = (Elf64_Ehdr *)elf->data;
    
    if (!elf_validate(elf->header)) {
        elf_free(elf);
        return -1;
    }
    
    //program headers
    if (elf->header->e_phoff != 0) {
        elf->program_headers =
            (Elf64_Phdr *)(elf->data + elf->header->e_phoff);
    }

    //section headers
    if (elf->header->e_shoff != 0) {
        elf->section_headers =
            (Elf64_Shdr *)(elf->data + elf->header->e_shoff);
    }

    /*******************************************************
        Section header string table
        .text
        .data
        .rodata
        .symtab
        etc.
    *********************************************************/
    if (elf->section_headers &&
        elf->header->e_shstrndx != SHN_UNDEF) {

        Elf64_Shdr *string_table =
            &elf->section_headers[elf->header->e_shstrndx];

        elf->section_names =
            (char *)(elf->data + string_table->sh_offset);
    }

    return 0;
}

void elf_free(ElfFile *elf) {
    if (!elf) return;

    free(elf->data);
    memset(elf, 0, sizeof(*elf));
}

void elf_print_header(const ElfFile *elf) {
    const Elf64_Ehdr *h = elf->header;

    printf("ELF Header\n");
    printf("==========\n");

    printf("Class:       ELF64\n");
    printf("Type:        %u\n", h->e_type);
    printf("Machine:     0x%x\n", h->e_machine);
    printf("Entry point: 0x%lx\n", h->e_entry);

    printf(
        "Program headers: %u\n",
        h->e_phnum
    );

    printf(
        "Section headers: %u\n",
        h->e_shnum
    );

    printf(
        "Program header offset: 0x%lx\n",
        h->e_phoff
    );

    printf(
        "Section header offset: 0x%lx\n",
        h->e_shoff
    );
}

void elf_print_sections(const ElfFile *elf) {
    const Elf64_Ehdr *h = elf->header;

    printf("\nSections\n");
    printf("========\n");

    for (int i = 0; i < h->e_shnum; i++) {

        const Elf64_Shdr *section =
            &elf->section_headers[i];

        const char *name =
            elf->section_names + section->sh_name;

        printf(
            "[%2d] %-20s "
            "addr=0x%016lx "
            "offset=0x%016lx "
            "size=0x%016lx\n",

            i,
            name,
            section->sh_addr,
            section->sh_offset,
            section->sh_size
        );
    }
}

void elf_print_program_headers(const ElfFile *elf)  {
    const Elf64_Ehdr *h = elf->header;

    printf("\nProgram Headers\n");
    printf("================\n");

    for (int i = 0; i < h->e_phnum; i++) {

        const Elf64_Phdr *program =
            &elf->program_headers[i];

        printf(
            "[%2d] type=%u "
            "offset=0x%lx "
            "vaddr=0x%lx "
            "filesz=0x%lx "
            "memsz=0x%lx\n",

            i,
            program->p_type,
            program->p_offset,
            program->p_vaddr,
            program->p_filesz,
            program->p_memsz
        );
    }
}

Elf64_Shdr *elf_find_section(
    const ElfFile *elf,
    const char *name
) {
    const Elf64_Ehdr *h = elf->header;

    for (int i = 0; i < h->e_shnum; i++) {

        Elf64_Shdr *section =
            &elf->section_headers[i];

        const char *section_name =
            elf->section_names + section->sh_name;

        if (strcmp(section_name, name) == 0) {
            return section;
        }
    }

    return NULL;
}
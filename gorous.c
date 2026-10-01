#include "include/elf.h"
#include<stdio.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <elf>\n", argv[0]);
        return 1;
    }
    
    ElfFile elf;
    
    if (elf_load(argv[1], &elf) != 0) return 1;
    
    elf_print_header(&elf);
    elf_print_sections(&elf);
    elf_print_program_headers(&elf);

    Elf64_Shdr *text =
        elf_find_section(&elf, ".text");

    if (text) {
        printf(
            "\n.text found:\n"
            "address: 0x%lx\n"
            "offset:  0x%lx\n"
            "size:    %lu bytes\n",

            text->sh_addr,
            text->sh_offset,
            text->sh_size
        );
    }

    elf_free(&elf);

    return 0;
}
#include "elf.h"

elfhdr_t gen_elfhdr() {
    return (elfhdr_t) {
        .ident = {
            0x7f,
            'E',
            'L',
            'F',
            0x02, // class 64
            0x01, // data LSB
            0x01, // version
            0x00, // SYSV
            0, 0, 0, 0, 0, 0, 0, 0
        },
        .type = 0x01, // REL
        .machine = 0x3e, // x86_64
        .version = 0x01,
        .entry = 0,
        .phoff = 0,
        .shoff = sizeof(elfhdr_t),
        .flags = 0,
        .hsize = sizeof(elfhdr_t),
        .phsize = 0,
        .phnum = 0,
        .shsize = sizeof(shdr_t),
        .shnum = 0,
        .strndx = 0,
    };
}

shdr_t gen_null_shdr() {
    return (shdr_t) {
        .name = 0,
        .type = 0,
        .flags = 0,
        .addr = 0,
        .offset = 0,
        .size = 0,
        .link = 0,
        .info = 0,
        .align = 0,
        .esize = 0,
    };
}

shdr_t gen_text_shdr() {
    return (shdr_t) {
        .name = 0,
        .type = 0x01, // DATA
        .flags = 0x02 | 0x04, // AX (ALLOC + EX)
        .addr = 0,
        .offset = 0,
        .size = 0,
        .link = 0,
        .info = 0,
        .align = 0x01,
        .esize = 0,
    };
}

shdr_t gen_strtab_shdr() {
    return (shdr_t) {
        .name = 0,
        .type = 0x03, // STRTAB
        .flags = 0,
        .addr = 0,
        .offset = 0,
        .size = 0,
        .link = 0,
        .info = 0,
        .align = 0x01,
        .esize = 0,
    };
}

shdr_t gen_symtab_shdr() {
    return (shdr_t) {
        .name = 0,
        .type = 0x02, // SYMTAB
        .flags = 0,
        .addr = 0,
        .offset = 0,
        .size = 0,
        .link = 0,
        .info = 0,
        .align = 0x08,
        .esize = sizeof(sym_t),
    };
}

size_t append_strtab(strtab_t *stab, const char *data) {
    size_t idx = stab->len;

    /* while (*data != '\0') { */
    /*     stab->buf[stab->len++] = *(data++); */
    /*     printf("DATA %d\n", stab->buf[stab->len-1]); */
    /* } */
    strcpy(stab->buf + stab->len, data);
    stab->len += strlen(data);
    stab->buf[stab->len++] = '\0';

    return idx;
}

elf64_t gen_elf64() {
    elf64_t elf;

    elf.hdr = gen_elfhdr();
    elf.hdr.shnum = 0x05;
    elf.hdr.strndx = 0x02;

    elf.sec[0] = (section_t) { .hdr = gen_null_shdr(), .data = 0, .len = 0};
    elf.sec[1] = (section_t) {.hdr = gen_text_shdr(), .data = 0, .len = 0 };
    elf.sec[2] = (section_t) {.hdr = gen_strtab_shdr(), .data = 0, .len = 0 };
    elf.sec[3] = (section_t) {.hdr = gen_symtab_shdr(), .data = 0, .len = 0 };
    elf.sec[4] = (section_t) {.hdr = gen_strtab_shdr(), .data = 0, .len = 0 };

    strtab_t stab = { .len = 0, .buf = malloc(MAX_BUF_SIZE) };
    elf.sec[0].hdr.name = append_strtab(&stab, ""); // null section
    elf.sec[1].hdr.name = append_strtab(&stab, ".text");
    elf.sec[2].hdr.name = append_strtab(&stab, ".shstrtab");
    elf.sec[3].hdr.name = append_strtab(&stab, ".symtab");
    elf.sec[4].hdr.name = append_strtab(&stab, ".strtab");

    elf.sec[2].data = stab.buf;
    elf.sec[2].len = stab.len;

    // TODO: avoid hardcoded symbols
    strtab_t symstab = { .len = 0, .buf = malloc(MAX_BUF_SIZE) };

    sym_t symtext = {
        .name = append_strtab(&symstab, ".text"),
        .info = 0x03, // SECTION
        .index = 0x01,
        .other = 0,
        .value = 0,
        .size = 0,
    };
    sym_t symstart = {
        .name = append_strtab(&symstab, "_start"),
        .info = 0x10, // GLOBAL
        .index = 0x01,
        .other = 0,
        .value = 0,
        .size = 0,
    };

    elf.sec[3].hdr.info = 0x01; // one symbol
    elf.sec[3].hdr.link = 0x04; // linked to strtab section

    elf.sec[3].len = sizeof(symstart) + sizeof(symtext);
    elf.sec[3].data = malloc(elf.sec[3].len);
    memcpy(elf.sec[3].data, &symtext, sizeof(symtext));
    memcpy(elf.sec[3].data + sizeof(symtext), &symstart, sizeof(symstart));

    elf.sec[4].data = symstab.buf;
    elf.sec[4].len = symstab.len;

    return elf;
}


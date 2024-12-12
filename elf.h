#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF_SIZE 1024

typedef struct {
    unsigned char ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t hsize;
    uint16_t phsize;
    uint16_t phnum;
    uint16_t shsize;
    uint16_t shnum;
    uint16_t strndx;
} elfhdr_t;

typedef struct {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t align;
    uint64_t esize;
} shdr_t;

typedef struct {
    uint32_t name;
    uint8_t info;
    uint8_t other;
    uint16_t index;
    uint64_t value;
    uint64_t size;
} sym_t;

typedef struct {
    shdr_t hdr;
    size_t len;
    void *data;
} section_t;

typedef struct {
    elfhdr_t hdr;
    section_t sec[5];
} elf64_t;

typedef struct {
    size_t len;
    /* char *buf[MAX_BUF_SIZE]; */
    char *buf;
} strtab_t;


elfhdr_t gen_elfhdr();
shdr_t gen_null_shdr();
shdr_t gen_text_shdr();
shdr_t gen_strtab_shdr();
shdr_t gen_symtab_shdr();
size_t append_strtab(strtab_t *stab, const char *data);
elf64_t gen_elf64();

#endif // ELF_H

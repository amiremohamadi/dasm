#include "asm.h"
#include "parser.h"

#define MAX_PROG_SIZE 1024

void collect_syms(asm_t *asmblr, elf64_t *elf, lexer_t *lex);
void encode_prog(asm_t *asmblr, elf64_t *elf, lexer_t *lex);
void update_off(elf64_t *elf);

void assemble(elf64_t *elf, lexer_t *lex) {
    asm_t asmblr = { .syms = NULL, .offset = 0 };

    collect_syms(&asmblr, elf, lex);
    encode_prog(&asmblr, elf, lex);
    update_off(elf);
}

void encode_prog(asm_t *asmblr, elf64_t *elf, lexer_t *lex) {
    instr_t ins;

    elf->sec[1].data = malloc(MAX_PROG_SIZE);
    do {
        ins = next_instr(lex);
        encode(asmblr, &ins, elf->sec[1].data);
    } while(ins.type != PARSER_EOF);
    elf->sec[1].len = asmblr->offset;
}

void update_off(elf64_t *elf) {
    size_t offset = sizeof(elfhdr_t) + (sizeof(shdr_t) * 5);

    for (int i = 1; i < 5; i++) {
        elf->sec[i].hdr.size = elf->sec[i].len;
        elf->sec[i].hdr.offset = offset;
        offset += elf->sec[i].len;
    }
}

void collect_syms(asm_t *asmblr, elf64_t *elf, lexer_t *lex) {
    instr_t ins;
    size_t offset = 0, num_syms = 0;
    symentry_t *prev = NULL;

    strtab_t symstab = { .len = 0, .buf = malloc(MAX_BUF_SIZE) };
    elf->sec[SYMTAB_SECTION].data = malloc(MAX_BUF_SIZE * sizeof(sym_t));

    // TODO: text section should be specified in the code
    sym_t symtext = {
        .name = append_strtab(&symstab, ".text"),
        .info = 0x03, // SECTION
        .index = 0x01,
        .other = 0, .value = 0, .size = 0,
    };
    memcpy(elf->sec[SYMTAB_SECTION].data, &symtext, sizeof(sym_t));
    num_syms++;

    do {
        ins = next_instr(lex);
        offset += instr_off(&ins);

        if (ins.type == INSTR_GLOBAL_LABEL) {
            sym_t gsym = {
                .name = append_strtab(&symstab, ((op_t *)ins.data)->data),
                .value = offset,
                .info = 0x10, // GLOBAL
                .index = 0x01,
                .size = 0,
                .other = 0,
            };
            memcpy(elf->sec[SYMTAB_SECTION].data + sizeof(sym_t) * num_syms,
                    &gsym, sizeof(sym_t));
            num_syms++;
        }

        if (ins.type == INSTR_LABEL) {
            symentry_t *next = NULL;

            next = malloc(sizeof(symentry_t));
            next->offset = offset;
            next->name = ins.data;
            next->next = NULL;

            if (prev) {
                prev->next = next;
                prev = prev->next;
                continue;
            }

            prev = next;
            asmblr->syms = prev;
        }
    } while (ins.type != PARSER_EOF);


    elf->sec[SYMTAB_SECTION].hdr.info = num_syms-1; // ignore text section
    elf->sec[SYMTAB_SECTION].len = num_syms * sizeof(sym_t);
    elf->sec[STRTAB_SECTION].data = symstab.buf;
    elf->sec[STRTAB_SECTION].len = symstab.len;

    lex->i = 0; // reset
}

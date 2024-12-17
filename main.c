#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "elf.h"

int read_file(const char *name, char **buf);
int output(lexer_t *lex, const char *name);
void out_lex(lexer_t *lex);
void out_parse(lexer_t *lex);

int main(int argc, char **argv) {

    lexer_t lex = { .buf = 0, .i = 0 };

    if (argc < 3) {
        printf("usage: ./dasm <filename> <flag> [output] (flag: --lex, --parse, --out)\n");
        return 1;
    }

    if (read_file(argv[1], &lex.buf)) {
        printf("could not open the file '%s'\n", argv[1]);
        return 1;
    }

    if (strcmp(argv[2], "--lex") == 0) {
        out_lex(&lex);
        goto cleanup;
    }

    if (strcmp(argv[2], "--parse") == 0) {
        out_parse(&lex);
        goto cleanup;
    }

    if (output(&lex, argv[3])) {

    }

cleanup:
    free(lex.buf);
    return 0;
}

int read_file(const char *name, char **buf) {
    FILE *f;
    size_t len;

    if ((f = fopen(name, "r")) == 0) {
        return 1;
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    *buf = (char *)malloc(len);
    fread(*buf, sizeof(char), len, f);
    *buf[len] = '\0';

    fclose(f);
    return 0;
}

int output(lexer_t *lex, const char *name) {
    FILE *f;
    instr_t ins;
    elf64_t elf = gen_elf64();

    size_t offset = 0;
    symentry_t *sym = NULL;
    asm_t asmblr = { .syms = NULL, .offset = 0 };

    // collect symbols
    do {
        ins = next_instr(lex);
        offset += instr_off(&ins);
        if (ins.type == INSTR_LABEL) {
            symentry_t *tmp = NULL;

            tmp = malloc(sizeof(symentry_t));
            tmp->offset = offset;
            tmp->name = ins.data;
            tmp->next = NULL;

            if (sym == NULL) {
                sym = tmp;
            } else {
                sym->next = tmp;
                sym = sym->next;
            }

            if (asmblr.syms == NULL) {
                asmblr.syms = sym;
            }
        }
    } while (ins.type != PARSER_EOF);
    lex->i = 0;

    // program
    elf.sec[1].data = malloc(1024);
    do {
        ins = next_instr(lex);
        encode(&asmblr, &ins, elf.sec[1].data);
    } while(ins.type != PARSER_EOF);
    elf.sec[1].len = asmblr.offset;

    // update offsets
    offset = sizeof(elfhdr_t) + (sizeof(shdr_t) * 5);
    for (int i = 1; i < 5; i++) {
        elf.sec[i].hdr.size = elf.sec[i].len;
        elf.sec[i].hdr.offset = offset;
        offset += elf.sec[i].len;
    }

    if ((f = fopen(name, "wb")) == 0) {
        return 1;
    }

    fwrite(&elf.hdr, sizeof(elf.hdr), 1, f);
    for (int i = 0; i < 5; i++) {
        fwrite(&elf.sec[i].hdr, sizeof(elf.sec[i].hdr), 1, f);
    }
    for (int i = 0; i < 5; i++) {
        if (elf.sec[i].len > 0) {
            fwrite(elf.sec[i].data, elf.sec[i].len, 1, f);
            free(elf.sec[i].data);
        }
    }

    fclose(f);
    return 0;
}

void out_lex(lexer_t *lex) {
    token_t tkn;

    do {
        tkn = next_token(lex);
        debug(tkn);
        FREE_TKN(tkn);
    } while (tkn.type != TKN_EOF);
}

void out_parse(lexer_t *lex) {
    instr_t ins;

    do {
        ins = next_instr(lex);
        FREE_INST(ins);
    } while (ins.type != PARSER_EOF);
}

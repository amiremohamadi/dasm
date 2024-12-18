#ifndef ASM_H
#define ASM_H

#include "lexer.h"
#include "elf.h"

void assemble(elf64_t *elf, lexer_t *lex);

#endif // ASM_H

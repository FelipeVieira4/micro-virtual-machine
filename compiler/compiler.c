#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "compiler.h"
#include "../utils/mystdint.h"
#include "../opcode_table.h"

#define DELIMS " ,\t\r\n"
#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

u32 generate_opcode(u8 opcode, u16 condic1, u16 condic2);

static bool lookup_opcode(const char *name, u8 *out) {
  for (size_t i = 0; i < ARRAY_LEN(opcode_mapper); i++) {
    if (strcmp(name, opcode_mapper[i].str_name) == 0) {
      *out = opcode_mapper[i].opcode_value;
      return true;
    }
  }
  return false;
}

/* aceita NULL, nome de registrador, decimal ou 0xHEX */
static bool parse_operand(const char *s, u16 *out) {
  if (!s) { *out = 0; return true; }

  for (size_t i = 0; i < ARRAY_LEN(registers_mapper); i++) {
    if (strcmp(s, registers_mapper[i].str_name) == 0) {
      *out = registers_mapper[i].opcode_value;
      return true;
    }
  }

  char *end;
  long v = strtol(s, &end, 0);
  if (end == s || *end != '\0') return false;   /* lixo no operando */
  *out = (u16)v;
  return true;
}

bool compile_file(char *patch_file, char *binary_file_output) {
  FILE *source_file = fopen(patch_file, "r");
  if (!source_file) {
    printf("[ERROR] Open File: %s\n", patch_file);
    return false;
  }

  FILE *out = fopen(binary_file_output, "wb");
  if (!out) {
    printf("[ERROR] Open File: %s\n", binary_file_output);
    fclose(source_file);
    return false;
  }

  char buffer[75];
  int lineno = 0;
  bool ok = true;

  while (fgets(buffer, sizeof buffer, source_file)) {
    lineno++;

    char *line = buffer;
    while (*line == ' ' || *line == '\t') line++;
    if (*line == '\0' || *line == '\n' || *line == '\r' || *line == ';')
      continue;

    char *opcode = strtok(line, DELIMS);
    char *arg1   = strtok(NULL, DELIMS);
    char *arg2   = strtok(NULL, DELIMS);
    if (!opcode) continue;

    u8  op;
    u16 c1, c2;

    if (!lookup_opcode(opcode, &op)) {
      printf("[ERROR] linha %d: opcode desconhecido '%s'\n", lineno, opcode);
      ok = false;
      break;
    }
    if (!parse_operand(arg1, &c1) || !parse_operand(arg2, &c2)) {
      printf("[ERROR] linha %d: operando invalido\n", lineno);
      ok = false;
      break;
    }

    u32 word = generate_opcode(op, c1, c2);
    fwrite(&word, sizeof word, 1, out);
  }

  fclose(out);
  fclose(source_file);
  return ok;
}

u32 generate_opcode(u8 opcode, u16 condic1, u16 condic2) {
  return ((u32)(opcode & 0xFF) << 24) |
         ((u32)(condic1 & 0xFFF) << 12) |
          (u32)(condic2 & 0xFFF);
}

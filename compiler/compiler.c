#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
 
#include "compiler.h"
#include "../utils/mystdint.h"
#include "../opcode_table.h"
 
#define DELIMS " ,\t\r\n"
#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define LINE_MAX_LEN 256
#define FIELD12_MAX  4095   /* maior valor que cabe em 12 bits */
 
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
 
/* aceita NULL, nome de registrador, decimal ou 0xHEX (0 a 4095) */
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
  if (v < 0 || v > FIELD12_MAX) return false;   /* nao cabe em 12 bits */
  *out = (u16)v;
  return true;
}
 
/* grava sempre em little-endian, independente da maquina */
static bool write_u32_le(FILE *f, u32 w) {
  u8 b[4] = {
    (u8)(w & 0xFF),
    (u8)((w >> 8) & 0xFF),
    (u8)((w >> 16) & 0xFF),
    (u8)((w >> 24) & 0xFF)
  };
  return fwrite(b, 1, 4, f) == 4;
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
 
  char buffer[LINE_MAX_LEN];
  int lineno = 0;
  bool ok = true;
 
  while (fgets(buffer, sizeof buffer, source_file)) {
    lineno++;
 
    /* linha maior que o buffer: erro em vez de quebrar em duas */
    size_t len = strlen(buffer);
    if (len == sizeof buffer - 1 && buffer[len - 1] != '\n' && !feof(source_file)) {
      printf("[ERROR] linha %d: maior que %d caracteres\n", lineno, LINE_MAX_LEN - 1);
      ok = false;
      break;
    }
 
    /* remove comentario ate o fim da linha */
    char *comment = strchr(buffer, ';');
    if (comment) *comment = '\0';
 
    char *opcode = strtok(buffer, DELIMS);
    if (!opcode) continue;                      /* linha vazia ou so comentario */
    char *arg1 = strtok(NULL, DELIMS);
    char *arg2 = strtok(NULL, DELIMS);
 
    if (strtok(NULL, DELIMS)) {
      printf("[ERROR] linha %d: operandos demais\n", lineno);
      ok = false;
      break;
    }
 
    u8  op;
    u16 c1, c2;
 
    if (!lookup_opcode(opcode, &op)) {
      printf("[ERROR] linha %d: opcode desconhecido '%s'\n", lineno, opcode);
      ok = false;
      break;
    }
    if (!parse_operand(arg1, &c1) || !parse_operand(arg2, &c2)) {
      printf("[ERROR] linha %d: operando invalido (use registrador ou 0 a %d)\n",
             lineno, FIELD12_MAX);
      ok = false;
      break;
    }
 
    if (!write_u32_le(out, generate_opcode(op, c1, c2))) {
      printf("[ERROR] falha ao escrever em %s\n", binary_file_output);
      ok = false;
      break;
    }
  }
 
  fclose(out);
  fclose(source_file);
 
  if (!ok) remove(binary_file_output);          /* nao deixa binario incompleto */
  return ok;
}
 
u32 generate_opcode(u8 opcode, u16 condic1, u16 condic2) {
  return ((u32)(opcode & 0xFF) << 24) |
         ((u32)(condic1 & 0xFFF) << 12) |
          (u32)(condic2 & 0xFFF);
}

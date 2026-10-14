#include <iostream>
#include <optional>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "compiler.hpp"
#include "../utils/mystdint.h"
#include "../opcode_table.h"

using namespace std;

/* ------------------------------------------------------------------ */
/* operandos e codificacao                                            */
/* ------------------------------------------------------------------ */

/* aceita NULL, registrador, numero (0 a 4095, decimal ou 0xHEX) ou label */

static bool parse_operand(const char *s, u16 *out,CLabelIndexer *labelIndexer) {
  if (!s) { *out = 0; return true; }

  if (lookup_register(s, out)) return true;

  char *end;
  long v = strtol(s, &end, 0);
  if (end != s && *end == '\0') {
    if (v < 0 || v > FIELD12_MAX) return false;
    *out = (u16)v;
    return true;
  }

  /*
  const label_s *l = labels ? find_label(labels, s) : NULL;
  if (!l || l->memory_location > FIELD12_MAX) return false;
  *out = (u16)l->memory_location;
  */

  optional<u32> memory_position=labelIndexer->get_memory_position(s);
  if (memory_position.has_value() && memory_position.value() < FIELD12_MAX){
    *out = (u16)memory_position.value();
  }else{
    return false;
  }

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

u32 generate_opcode(u8 opcode, u16 condic1, u16 condic2) {
  return ((u32)(opcode & 0xFF) << 24) |
         ((u32)(condic1 & 0xFFF) << 12) |
          (u32)(condic2 & 0xFFF);
}

/* ------------------------------------------------------------------ */
/* uma passada sobre o arquivo                                        */
/*   pass 1: coleta labels e conta instrucoes (out == NULL)           */
/*   pass 2: resolve operandos e escreve o binario                    */
/* ------------------------------------------------------------------ */

static bool run_pass(FILE *src, FILE *out,CLabelIndexer *labelIndexer, int pass) {
  char buffer[LINE_MAX_LEN];
  int  lineno = 0;
  u32  instr_index = 0;                          /* instrucoes vistas ate agora */

  while (fgets(buffer, sizeof buffer, src)) {
    lineno++;

    /* linha maior que o buffer: erro em vez de quebrar em duas */
    size_t len = strlen(buffer);
    if (len == sizeof buffer - 1 && buffer[len - 1] != '\n' && !feof(src)) {
      printf("[ERROR] linha %d: maior que %d caracteres\n", lineno, LINE_MAX_LEN - 1);
      return false;
    }

    /* remove comentario ate o fim da linha */
    char *comment = strchr(buffer, ';');
    if (comment) *comment = '\0';

    char *tok = strtok(buffer, DELIMS);

    /* zero ou mais labels no comeco da linha: "loop:" ou "loop: INC R0" */
    while (tok && tok[strlen(tok) - 1] == ':') {
      tok[strlen(tok) - 1] = '\0';

      if (pass == 1 && !labelIndexer->push_new_label(tok,instr_index)) {
        printf("[ERROR] linha %d: implementação da label '%s'\n", lineno, tok);
        return false;
      }
      tok = strtok(NULL, DELIMS);
    }
    if (!tok) continue;                          /* vazia, comentario ou so label */

    char *arg1 = strtok(NULL, DELIMS);
    char *arg2 = strtok(NULL, DELIMS);
    if (strtok(NULL, DELIMS)) {
      printf("[ERROR] linha %d: operandos demais\n", lineno);
      return false;
    }

    u8 op;
    if (!lookup_opcode(tok, &op)) {
      printf("[ERROR] linha %d: opcode desconhecido '%s'\n", lineno, tok);
      return false;
    }

    if (pass == 1) { instr_index++; continue; }  /* so conta */

    u16 c1, c2;
    if (!parse_operand(arg1, &c1, labelIndexer) || !parse_operand(arg2, &c2, labelIndexer)) {
      printf("[ERROR] linha %d: operando invalido ou label inexistente (0 a %d)\n",
             lineno, FIELD12_MAX);
      return false;
    }
    if (!write_u32_le(out, generate_opcode(op, c1, c2))) {
      printf("[ERROR] linha %d: falha ao escrever\n", lineno);
      return false;
    }
    instr_index++;
  }
  return true;
}

bool compile_file(char *patch_file, char *binary_file_output) {
  CLabelIndexer labelIndexer = CLabelIndexer();

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

  bool status = run_pass(source_file, NULL, &labelIndexer, 1);   // gerar labels
  if (status) {
    rewind(source_file);
    status = run_pass(source_file, out, &labelIndexer, 2);       // gerar binario
  }

  fclose(out);
  fclose(source_file);

  if (!status) remove(binary_file_output);                 /* nao deixa binario incompleto */
  return status;
}
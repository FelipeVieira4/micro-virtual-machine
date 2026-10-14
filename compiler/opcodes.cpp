#include <string.h>

#include "compiler.hpp"
#include "../opcode_table.h"
#include "../utils/mystdint.h"

/*
  Opcodes Types:

  [0] Default:
  OPC[8 bit] Cond1 [12 bit] Cond2[12 bit]

  [1] for Jumps and Calls
  OPC[8 bit] Cond1 [21 bit] Cond2[3 bit]

  [2] (in Future)
  for LOCAL, STORE etc...
  OPC[8 bit] Cond1 [4 bit] Cond2[20 bit]
*/

/* ------------------------------------------------------------------ */
/* opcodes / registradores                                            */
/* ------------------------------------------------------------------ */

bool lookup_opcode(const char *name, u8 *out) {
  for (size_t i = 0; i < ARRAY_LEN(opcode_mapper); i++) {
    if (strcmp(name, opcode_mapper[i].str_name) == 0) {
      *out = opcode_mapper[i].opcode_value;
      return true;
    }
  }
  return false;
}

bool lookup_register(const char *name, u16 *out) {
  for (size_t i = 0; i < ARRAY_LEN(registers_mapper); i++) {
    if (strcmp(name, registers_mapper[i].str_name) == 0) {
      *out = registers_mapper[i].opcode_value;
      return true;
    }
  }
  return false;
}

bool is_reserved_name(const char *name) {
  u8  dummy_op;
  u16 dummy_reg;
  return lookup_opcode(name, &dummy_op) || lookup_register(name, &dummy_reg);
}
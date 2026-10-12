#ifndef _OPCODE_TABLE_H_
#define _OPCODE_TABLE_H_

#include "utils/mystdint.h"

typedef enum {
    OPC_NOP      = 0x00,
    OPC_MOV, // Registers opcodes
    OPC_SET,
    OPC_LOAD,
    OPC_STORE,

    OPC_INC      = 0x10, // Arithmetic opcodes
    OPC_DEC,
    OPC_DIV,
    OPC_MUL,
    OPC_ADD,
    OPC_SUB,

    OPC_CMP      = 0x1C, // Condition opcodes
    OPC_GT,
    OPC_LT,

    OPC_CALL     = 0x26, // Control flow opcodes
    OPC_JMP,
    OPC_RETURN,

    OPC_SYS_CALL = 0x2C, // System calls
    OPC_SYS_EXIT,

    NUM_OPC
} opcode_enum;

typedef struct {
    const char *str_name;
    u8 opcode_value;
} mapper_s;

static const mapper_s opcode_mapper[] = {
	{"NOP", OPC_NOP},
    {"MOV",      OPC_MOV},
    {"SET",      OPC_SET},
    {"LOAD",     OPC_LOAD},
    {"STORE",    OPC_STORE},

    {"INC",      OPC_INC},
    {"DEC",      OPC_DEC},
    {"DIV",      OPC_DIV},
    {"MUL",      OPC_MUL},
	{"ADD",      OPC_ADD},
    {"SUB",      OPC_SUB},

    {"CMP",      OPC_CMP},
    {"GT",       OPC_GT},
    {"LT",       OPC_LT},

    {"CALL",     OPC_CALL},
    {"JMP",      OPC_JMP},
    {"RETURN",   OPC_RETURN},

    {"SYS_CALL", OPC_SYS_CALL},
    {"SYS_EXIT", OPC_SYS_EXIT},
};


typedef enum {
  R0 = 0x00,
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  NUM_REGISTERS
} registers_enum;

static const mapper_s registers_mapper[] = {
    {"R0", R0},
    {"R1", R1},
    {"R2", R2},
    {"R3", R3},
    {"R4", R4},
    {"R5", R5},
    {"R6", R6},
    {"R7", R7},
};

#endif

#ifndef _VM_H_
#define _VM_H_

#include "stdbool.h"

#include "utils/mystdint.h"

#define MEMORY_SIZE 4096
#define PROGRAM_SIZE 512 // byts
#define REGISTER_NUMBER 8
#define MAX_STACK 32

typedef struct {
  u32 opcode;
  u16 pc;

  u8  memory[MEMORY_SIZE];
  u16 registers[REGISTER_NUMBER];   // u16: cabe imediato de 12 bits

  u8 *program;

  u16 stack_pointer;
  u16 stack[MAX_STACK];

  u32 program_size;
  bool running;
  bool flag;                        // resultado de CMP/GT/LT
} vm_s;

bool init_vm(vm_s *vm, char *binary_name);
bool cycle_vm(vm_s *vm);
void free_vm(vm_s *vm);
void print_opcode(vm_s *vm);
#endif

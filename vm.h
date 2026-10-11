#ifndef _VM_H_
#define _VM_H_

#include "stdbool.h"

#include "utils/mystdint.h"

#define MEMORY_SIZE 4096
#define PROGRAM_SIZE 512 // byts
#define REGISTER_NUMBER 12
#define MAX_STACK 64

typedef struct {
  u32 opcode; // uses 32 bits to make easily the conversion for 16 and 32 bits
  u16 pc;
  
  u8 memory[MEMORY_SIZE];
  u8 registers[REGISTER_NUMBER];

  u8 *program;
  
  u16 stack_pointer;
  u16 stack[MAX_STACK];

  u32 program_size;
  bool running;
}vm_s;

bool init_vm(vm_s *vm, char *binary_name);
bool cycle_vm(vm_s *vm);


// functions for debug
void print_opcode(vm_s *vm);

#endif

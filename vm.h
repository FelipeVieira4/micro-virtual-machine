#ifndef _VM_H_
#define _VM_H_

#include "stdbool.h"

#include "utils/mystdint.h"

#define MEMORY_SIZE 4096
#define REGISTER_NUMBER 12
#define MAX_STACK 64

typedef struct {
  u16 opcode;
  u16 pc;
  
  u8 memory[MEMORY_SIZE];
  u8 registers[REGISTER_NUMBER];

  u16 stack_pointer;
  u16 stack[MAX_STACK];

  bool running;
}vm_s;

bool init_vm(vm_s *vm,const char *binary_name);
bool cycle_vm(vm_s *vm);

#endif

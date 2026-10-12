#include <stdbool.h>
#include <stdio.h>

#include "fault.h"
#include "../vm.h"

bool vm_fault(vm_s *vm, const char *msg) {
  printf("[VM - FAULT ERROR] pc=0x%04X: %s\n", (unsigned)(vm->pc - 4), msg);
  vm->running = false;
  return false;
}
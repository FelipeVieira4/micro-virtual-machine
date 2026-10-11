#include "vm.h"

#include <string.h>

#include "utils/mystdint.h"

bool init_vm(vm_s *vm, const char *binary_name) {
  memset(vm,0,sizeof(*vm));

  return true;
}

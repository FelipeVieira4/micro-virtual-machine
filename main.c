#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

int main(int argc, char *argv[]) {
  vm_s virtual_machine;

  if (argc < 2){
    printf("ROM não informado!");
  }

  if (!init_vm(&virtual_machine,argv[1])) return 0;

  cycle_vm(&virtual_machine);
  cycle_vm(&virtual_machine);
  cycle_vm(&virtual_machine);
  
  return 1;
}

#ifndef _FAULT_H_
#define _FAULT_H_

#include <stdbool.h>

#include "mystdint.h"
#include "../vm.h"

#define ERROR_MSG 0x01
#define WARNING_MSG 0x02
#define SUCESS_MSG 0x03

bool vm_fault(vm_s *vm, const char *msg);
#endif
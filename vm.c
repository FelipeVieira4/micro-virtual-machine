#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "utils/mystdint.h"
#include "utils/fault.h"

#include "vm.h"
#include "opcode_table.h"

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

bool load_program(char *patch_file, vm_s *vm) {
    FILE *program_file = fopen(patch_file, "rb");
    if (!program_file) {
        printf("[ERROR] Erro ao abrir a ROM: %s\n", patch_file);
        return false;
    }
 
    if (fseek(program_file, 0, SEEK_END) != 0) {
        printf("[ERROR] fseek falhou\n");
        fclose(program_file);
        return false;
    }
 
    long size = ftell(program_file);          /* long: -1 em caso de erro */
    if (size < 0) {
        printf("[ERROR] ftell falhou\n");
        fclose(program_file);
        return false;
    }
    rewind(program_file);
 
    if (size == 0) {
        printf("[ERROR] Arquivo vazio\n");
        fclose(program_file);
        return false;
    }
    if (size % 4 != 0) {                      /* instrucoes de 4 bytes */
        printf("[ERROR] Tamanho invalido (%ld bytes): deve ser multiplo de 4\n", size);
        fclose(program_file);
        return false;
    }

    /*
    if ((unsigned long)size > MAX_PROGRAM_SIZE) {
        printf("[ERROR] Programa grande demais (%ld bytes, maximo %u)\n",
               size, MAX_PROGRAM_SIZE);
        fclose(program_file);
        return false;
    }
    */
 
    vm->program = malloc((size_t)size);
    if (vm->program == NULL) {
        printf("[ERROR] Alocacao da memoria falhou\n");
        fclose(program_file);
        return false;
    }
 
    size_t bytes_read = fread(vm->program, 1, (size_t)size, program_file);
    fclose(program_file);
 
    if (bytes_read != (size_t)size) {
        printf("[ERROR] Erro ao ler o arquivo completo\n");
        free(vm->program);
        vm->program = NULL;                   /* evita ponteiro pendurado / double free */
        return false;
    }
 
    vm->program_size = (u32)size;
    return true;
}

bool init_vm(vm_s *vm, char *binary_name) {
  memset(vm, 0, sizeof(*vm));
  if (!load_program(binary_name, vm)) return false;
  vm->running = true;
  return true;
}

void free_vm(vm_s *vm) {
  free(vm->program);
  vm->program = NULL;
}

static bool jump_to(vm_s *vm, u16 instr_index) {
  u32 target = (u32)instr_index * 4;
  if (target >= vm->program_size) return vm_fault(vm, "salto fora do programa");
  vm->pc = (u16)target;
  return true;
}

static bool do_syscall(vm_s *vm, u16 num, u16 val) {
  switch (num) {
  case 0: printf("%u\n", (unsigned)val); return true;
  case 1: putchar((int)(val & 0xFF)); fflush(stdout); return true;
  case 2:
#ifdef ESP_PLATFORM
    vTaskDelay(pdMS_TO_TICKS(val));
#endif
    return true;
  default: return vm_fault(vm, "syscall invalida");
  }
}

#define CHECK_REG(r) do { if ((r) >= REGISTER_NUMBER) \
    return vm_fault(vm, "registrador invalido"); } while (0)
#define R(x) (vm->registers[(x)])

bool cycle_vm(vm_s *vm) {
  if (!vm->running) return false;

  if ((u32)vm->pc + 3 >= vm->program_size) {
    vm->running = false;
    return false;
  }

  const u8 *p = &vm->program[vm->pc];
  vm->opcode = (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
  vm->pc += 4;

  u8  op = (vm->opcode >> 24) & 0xFF;
  u16 c1 = (vm->opcode >> 12) & 0xFFF;
  u16 c2 =  vm->opcode        & 0xFFF;

  switch (op) {
  case OPC_NOP:
    break;

  /* ---- registradores / memoria ---- */
  case OPC_MOV:   CHECK_REG(c1); CHECK_REG(c2); R(c1) = R(c2); break;
  case OPC_SET:   CHECK_REG(c1); R(c1) = c2; break;
  case OPC_LOAD:
    CHECK_REG(c1); CHECK_REG(c2);
    if (R(c2) >= MEMORY_SIZE) return vm_fault(vm, "LOAD fora da memoria");
    R(c1) = vm->memory[R(c2)];
    break;
  case OPC_STORE:
    CHECK_REG(c1); CHECK_REG(c2);
    if (R(c1) >= MEMORY_SIZE) return vm_fault(vm, "STORE fora da memoria");
    vm->memory[R(c1)] = (u8)(R(c2) & 0xFF);
    break;

  /* ---- aritmetica ---- */
  case OPC_INC:   CHECK_REG(c1); R(c1)++; break;
  case OPC_DEC:   CHECK_REG(c1); R(c1)--; break;
  case OPC_ADD:   CHECK_REG(c1); CHECK_REG(c2); R(c1) += R(c2); break;
  case OPC_SUB:   CHECK_REG(c1); CHECK_REG(c2); R(c1) -= R(c2); break;
  case OPC_MUL:   CHECK_REG(c1); CHECK_REG(c2); R(c1) *= R(c2); break;
  case OPC_DIV:
    CHECK_REG(c1); CHECK_REG(c2);
    if (R(c2) == 0) return vm_fault(vm, "divisao por zero");
    R(c1) /= R(c2);
    break;

  /* ---- condicoes ---- */
  case OPC_CMP:   CHECK_REG(c1); CHECK_REG(c2); vm->flag = (R(c1) == R(c2)); break;
  case OPC_GT:    CHECK_REG(c1); CHECK_REG(c2); vm->flag = (R(c1) >  R(c2)); break;
  case OPC_LT:    CHECK_REG(c1); CHECK_REG(c2); vm->flag = (R(c1) <  R(c2)); break;

  /* ---- fluxo de controle ---- */
  case OPC_JMP:
    if (c2 == 0 || (c2 == 1 && vm->flag) || (c2 == 2 && !vm->flag)) {
      if (!jump_to(vm, c1)) return false;
    } else if (c2 > 2) {
      return vm_fault(vm, "condicao de JMP invalida");
    }
    break;

  case OPC_CALL:
    if (vm->stack_pointer >= MAX_STACK) return vm_fault(vm, "stack overflow");
    vm->stack[vm->stack_pointer++] = vm->pc;   // pc ja aponta p/ proxima instrucao
    if (!jump_to(vm, c1)) return false;
    break;

  case OPC_RETURN:
    if (vm->stack_pointer == 0) return vm_fault(vm, "stack underflow");
    vm->pc = vm->stack[--vm->stack_pointer];
    break;

  /* ---- sistema ---- */
  case OPC_SYS_CALL:
    CHECK_REG(c2);
    if (!do_syscall(vm, c1, R(c2))) return false;
    break;

  case OPC_SYS_EXIT:
    vm->running = false;
    break;

  default:
    return vm_fault(vm, "opcode desconhecido");
  }

  return true;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "utils/mystdint.h"


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


bool init_vm(vm_s *vm,char *binary_name) {
  memset(vm,0,sizeof(*vm));
  
  if(!load_program(binary_name,vm)) return false;
    
  return true;
}

void print_opcode(vm_s *vm) {
  u8  op = (vm->opcode >> 24) & 0xFF;
  u16 c1 = (vm->opcode >> 12) & 0xFFF;
  u16 c2 =  vm->opcode        & 0xFFF;

  printf("opcode: 0x%08X | op: 0x%02X | c1: 0x%03X | c2: 0x%03X\n", 
	 vm->opcode, op, c1, c2);
}


bool cycle_vm(vm_s *vm) {
  /* precisa de 4 bytes disponiveis */
  if ((u32)vm->pc + 3 >= vm->program_size) {
    vm->running = false;
    return false;
  }

  const u8 *p = &vm->program[vm->pc];

  /* little-endian: mesmo formato que o compilador grava */
  vm->opcode = (u32)p[0]
             | ((u32)p[1] << 8)
             | ((u32)p[2] << 16)
             | ((u32)p[3] << 24);

  vm->pc += 4;
  
  switch (vm->opcode & 0xFF) {
  case 0x0000:
    break;
  default:
    return false;
  }
  
  print_opcode(vm);
  return true;
}

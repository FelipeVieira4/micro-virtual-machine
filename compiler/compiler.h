#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <stdbool.h>

#include "../utils/mystdint.h"

#define MAX_LABELS     36
#define LABEL_NAME_MAX 32

typedef struct {
  char name[LABEL_NAME_MAX];
  u32  memory_location;      /* indice da instrucao (pc = indice * 4) */
} label_s;

typedef struct {
  label_s list[MAX_LABELS];
  size_t  count;
} label_table_s;

bool compile_file(char *patch_file, char *binary_file_output);


#endif

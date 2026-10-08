#include "compiler.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>


bool compile_file(char *patch_file, char *binary_file_output) {
  FILE *source_file = fopen(patch_file, "r");
  if (!source_file) {
    printf("[ERROR] Open File: %s\n", patch_file);
    return false;
  }
  
  int bufferLength = 75;
  char buffer[bufferLength]; /* not ISO 90 compatible */

  while (fgets(buffer, bufferLength, source_file)) {
    if (buffer[0] == '\n' || buffer[0] == '\r' ||
        buffer[0] == '\0' || buffer[0] == ';')
      continue;

    
    char *opcode = strtok(buffer," ");
    printf("Opcode:%s\n", opcode);

    char *condic1 = strtok(0,",");
    printf("Condic[0]:%s\n", condic1);

    char *condic2 = strtok(0,",");
    printf("Condic[1]:%s\n", condic2);


  }

  fclose(source_file);
  
  return true;
}

u32 generate_opcode(u8 opcode, u16 condic1, u16 condic2) {
  return ((u32)(opcode & 0xFF) << 24 |
          (u32)(condic1 & 0xFFF) << 12 |
	  (u32)(condic2 & 0xFFF));
}

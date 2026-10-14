#include <stdio.h>

#include "compiler.hpp"

int main(int argc,char *argv[]) {
  if (argc<3){
    printf("[ERROR] Arquivo de entrada e saída não informado!\n");
    return -1;  
  }

  if (compile_file(argv[1], argv[2])) return -1;

  return 0;
}

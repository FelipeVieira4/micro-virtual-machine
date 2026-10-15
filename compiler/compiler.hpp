#ifndef _COMPILER_HPP_
#define _COMPILER_HPP_

#include <iostream>
#include <optional>
#include <unordered_map>

#include "../utils/mystdint.h"

#define DELIMS " ,\t\r\n"
#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define LINE_MAX_LEN 256
#define FIELD12_MAX  4095   /* maior valor que cabe em 12 bits */


using namespace std;

class CLabelIndexer{
  public:
  unordered_map<string,u32> labels_list;  // label name, memory position

  bool push_new_label(const string& label_name,u32 memory_position);
  optional<u32> get_memory_position(const string& label_name);
};

// C functions for compiler

bool lookup_opcode(const char *name, u8 *out);
bool lookup_register(const char *name, u16 *out);
bool is_reserved_name(const char *name);

bool compile_file(char *patch_file, char *binary_file_output);

#endif

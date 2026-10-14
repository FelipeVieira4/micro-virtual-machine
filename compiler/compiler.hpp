#ifndef _COMPILER_HPP_
#define _COMPILER_HPP_

#include <iostream>
#include <optional>
#include <unordered_map>

#include "../utils/mystdint.h"

using namespace std;

bool compile_file(char *patch_file, char *binary_file_output);

class CLabelIndexer{
  public:
  unordered_map<string,u32> labels_list;  // label name, memory position

  bool push_new_label(const string& label_name,u32 memory_position);
  optional<u32> get_memory_position(const string& label_name);
};


#endif

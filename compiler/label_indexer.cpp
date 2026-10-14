#include <unordered_map>
#include <string>
#include <stdbool.h>

#include "compiler.hpp"

using namespace std;

/*
label nao pode ter o mesmo nome de opcode ou registrador
static bool is_reserved_name(const char *name) {
  u8  dummy_op;
  u16 dummy_reg;
  return lookup_opcode(name, &dummy_op) || lookup_register(name, &dummy_reg);
}
*/

bool CLabelIndexer::push_new_label(const string& label_name,u32 memory_position){
    if (label_name.empty()) return false;

    return this->labels_list.emplace(label_name,memory_position).second;
}

optional<u32> CLabelIndexer::get_memory_position(const string& label_name){
    auto it = this->labels_list.find(label_name);
    if (it == this->labels_list.end()) return std::nullopt;
    return it->second;
}
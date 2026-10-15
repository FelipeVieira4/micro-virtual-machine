#include <unordered_map>
#include <string>
#include <stdbool.h>

#include "compiler.hpp"

using namespace std;

bool CLabelIndexer::push_new_label(const string& label_name,u32 memory_position){
    if (label_name.empty()) return false;
    if (is_reserved_name(label_name.c_str())) return false;

    return this->labels_list.emplace(label_name,memory_position).second;
}

optional<u32> CLabelIndexer::get_memory_position(const string& label_name){
    auto it = this->labels_list.find(label_name);
    if (it == this->labels_list.end()) return std::nullopt;
    return it->second;
}
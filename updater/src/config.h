#pragma once

#include <filesystem>
#include <string>
#include <toml.hpp>

#include <fstream>

template <typename T>
inline T ConfigBind(toml::table &table, std::string key, T defaultValue) {
  if (table.contains(key)) {
    return table[key].value_or(defaultValue);
  }

  table.insert_or_assign(key, defaultValue);

  return defaultValue;
}

template <typename T>
void ConfigSet(toml::table &table, std::string key, T value) {
  table.insert_or_assign(key, value);
}

//
// Created by user on 27.12.2024.
//

#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <toml.hpp>
#include <utility>

class config {
public:
    static config load_mod_config(const std::string& mod_name);
    static config load(const std::string& path);

    void save();

    [[nodiscard]] bool has(const std::string& key) const { return table.contains(key); }

    template <typename T>
    T get(const std::string& key, T default_value) {
        return table[key].value_or(default_value);
    }

    template <typename T>
    void set(const std::string& key, T value) {
        table.insert_or_assign(key, value);

        save(); // Save the value to the file
    }

    template <typename T>
    T bind(const std::string& key, T default_value) {
        if (!has(key)) {
            set<T>(key, default_value);

            save(); // Save the default value to the file
            return default_value;
        }

        return get<T>(key, default_value);
    }

    toml::table& get_data() { return table; }
private:
    config(std::string path, toml::table table) : table(table), path(path) {}

    toml::table table;
    std::string path;
};

#endif //CONFIG_H
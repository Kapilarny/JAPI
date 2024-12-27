//
// Created by user on 27.12.2024.
//

#ifndef GAME_TYPE_H
#define GAME_TYPE_H

enum class game_type {
    UNKNOWN,
    ASBR,
    NSUNSC
};

inline std::string game_type_to_string(game_type type) {
    switch (type) {
        case game_type::ASBR:
            return "ASBR";
        case game_type::NSUNSC:
            return "NSUNSC";
        default:
            return "UNKNOWN";
    }
}

inline game_type string_to_game_type(const std::string& str) {
    std::string game_types[] = {"ASBR", "NSUNSC"};

    for(int i = 0; i < std::size(game_types); i++) {
        if (game_types[i] == str) {
            return static_cast<game_type>(i + 1);
        }
    }

    return game_type::UNKNOWN;
}

#endif //GAME_TYPE_H

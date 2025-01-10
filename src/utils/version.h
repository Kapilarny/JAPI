//
// Created by user on 03.01.2025.
//

#ifndef VERSION_H
#define VERSION_H
#include <cstdio>
#include <string>

namespace version {
    struct version {
        int major;
        int minor;
        int patch;
    };

    inline version parse_version(const char* version_str) {
        version ver{};
        sscanf(version_str, "%d.%d.%d", &ver.major, &ver.minor, &ver.patch);
        return ver;
    }

    inline bool is_version_greater(const version& a, const version& b) {
        if(a.major > b.major) {
            return true;
        }

        if(a.major == b.major) {
            if(a.minor > b.minor) {
                return true;
            }

            if(a.minor == b.minor) {
                return a.patch > b.patch;
            }
        }

        return false;
    }

    inline std::string version_to_string(const version& ver) {
        char* buf = new char[32];
        sprintf_s(buf, 32, "%d.%d.%d", ver.major, ver.minor, ver.patch);
        return {buf};
    }
}

#endif //VERSION_H

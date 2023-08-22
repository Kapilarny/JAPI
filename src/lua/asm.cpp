#include "asm.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <asmjit/x86.h>
#include <asmtk/asmtk.h>
#include "utils/logger.h"

using namespace asmtk;
using namespace asmjit;

static std::string dumpCode(const uint8_t* buf, size_t size) {
    std::string code;
    constexpr uint32_t kCharsPerLine = 39;
    char hex[kCharsPerLine * 2 + 1];

    size_t i = 0;
    while (i < size) {
        size_t j = 0;
        size_t end = size - i < kCharsPerLine ? size - i : size_t(kCharsPerLine);

        end += i;
        while (i < end) {
        uint8_t b0 = buf[i] >> 4;
        uint8_t b1 = buf[i] & 15;

        hex[j++] = b0 < 10 ? '0' + b0 : 'A' + b0 - 10;
        hex[j++] = b1 < 10 ? '0' + b1 : 'A' + b1 - 10;
        i++;
        }

        hex[j] = '\0';
        code += hex;
    }

    return code;
}

void ExecuteASMCode(const std::string& code) {
    Environment env;
    env.init(Arch::kX64);

    FileLogger logger(stdout);
    logger.addFlags(FormatFlags::kMachineCode);

    CodeHolder codeHolder;
    Error err = codeHolder.init(env);

    if(err) {
        JERROR("Failed to initialize code holder: " + std::string(DebugUtils::errorAsString(err)));
        return;
    }


    codeHolder.setLogger(&logger);
    x86::Assembler assembler(&codeHolder);
    AsmParser parser(&assembler);
    err = parser.parse(code.c_str());
    if(err) {
        JERROR("Failed to parse code: " + std::string(DebugUtils::errorAsString(err)));
        return;
    }

    JitRuntime jit;
    CodeBuffer& buffer = codeHolder.sectionById(0)->buffer();

    JINFO(dumpCode(buffer.data(), buffer.size()));

    typedef void (*Func)();
    Func fn;
    err = jit.add(&fn, &codeHolder);

    if(err) {
        JERROR("Failed to add code to JIT: " + std::string(DebugUtils::errorAsString(err)));
        return;
    }

    fn();

    JINFO("Successfully executed ASM code!");

    jit.release(fn);
}
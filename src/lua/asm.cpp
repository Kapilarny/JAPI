#include "asm.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <asmjit/x86.h>
#include <asmtk/asmtk.h>

#include "utils/config.h"
#include "utils/logger.h"

using namespace asmtk;
using namespace asmjit;

static Environment env;
static FileLogger logger(stdout);
bool shouldAttachLogger = false;

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

void InitASMExecutor() {
    auto config = GetModConfig("JAPI");
    shouldAttachLogger = ConfigBind(config.table, "dump_asm_code", false);
    SaveConfig(config);

    env.init(Arch::kX64);
    logger.addFlags(FormatFlags::kMachineCode);

    LOG_INFO("AsmExecutor", "Initialized ASM executor!");
}

__int64 AllocatedRetInstruction() {
    static CodeHolder codeHolder;
    static JitRuntime jit;
    static bool initialized = false;
    typedef void (*Func)();
    static Func fn;

    if(initialized) {
        return (__int64)fn;
    }

    Error err = codeHolder.init(env);
    if(err) {
        LOG_ERROR("AsmExecutor", "Failed to initialize code holder: " + std::string(DebugUtils::errorAsString(err)));
        return 0;
    }

    if(shouldAttachLogger) codeHolder.setLogger(&logger);

    x86::Assembler assembler(&codeHolder);
    AsmParser parser(&assembler);
    err = parser.parse("ret");
    if(err) {
        LOG_ERROR("AsmExecutor", "Failed to parse code: " + std::string(DebugUtils::errorAsString(err)));
        return 0;
    }

    err = jit.add(&fn, &codeHolder);
    if(err) {
        LOG_ERROR("AsmExecutor", "Failed to add code to JIT: " + std::string(DebugUtils::errorAsString(err)));
        return 0;
    }

    initialized = true;

    return AllocatedRetInstruction();
}

void ExecuteASMCode(const std::string& code) {
    CodeHolder codeHolder;
    Error err = codeHolder.init(env);

    if(err) {
        LOG_ERROR("AsmExecutor", "Failed to initialize code holder: " + std::string(DebugUtils::errorAsString(err)));
        return;
    }

    if(shouldAttachLogger) codeHolder.setLogger(&logger);

    x86::Assembler assembler(&codeHolder);
    AsmParser parser(&assembler);
    err = parser.parse(code.c_str());
    if(err) {
        LOG_ERROR("AsmExecutor", "Failed to parse code: " + std::string(DebugUtils::errorAsString(err)));
        return;
    }

    JitRuntime jit;
    CodeBuffer& buffer = codeHolder.sectionById(0)->buffer();

    typedef void (*Func)();
    Func fn;
    err = jit.add(&fn, &codeHolder);

    if(err) {
        LOG_ERROR("AsmExecutor", "Failed to add code to JIT: " + std::string(DebugUtils::errorAsString(err)));
        return;
    }

    fn();

    LOG_INFO("AsmExecutor", "Successfully executed ASM code!");

    jit.release(fn);
}
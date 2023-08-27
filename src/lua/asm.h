#pragma once

#include <string>

void InitASMExecutor();
__int64 AllocatedRetInstruction();
void ExecuteASMCode(const std::string& code);
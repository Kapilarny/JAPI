#include "bp_hook.h"

#include "utils/logger.h"

static bool s_handlerCreated = false;
static std::vector<BreakpointHook*> s_hookList;

BreakpointHook::BreakpointHook(std::uintptr_t addr, std::function<void(PEXCEPTION_POINTERS)> handler) {
	this->m_addr = addr;
	this->m_handler = handler;
	this->m_originalBytes = *(uint8_t*)m_addr;

	if (!s_handlerCreated) {
		JINFO("Creating VEH");
        AddVectoredExceptionHandler(true, (PVECTORED_EXCEPTION_HANDLER)onException);
		JINFO("VEH created");
    }

	JINFO("Pushing hook to list: " + std::to_string(addr));
	s_hookList.push_back(this);

	this->Enable();
}

void BreakpointHook::Enable() {
	VirtualProtect((PVOID)m_addr, 1, PAGE_EXECUTE_READWRITE, &m_originalProtection);
	*(uint8_t*)m_addr = 0xCC;
	VirtualProtect((PVOID)m_addr, 1, m_originalProtection, nullptr);
}

void BreakpointHook::Disable() {
	VirtualProtect((PVOID)m_addr, 1, PAGE_EXECUTE_READWRITE, nullptr);
	*(uint8_t*)m_addr = m_originalBytes;
	VirtualProtect((PVOID)m_addr, 1, m_originalProtection, nullptr);
}

BreakpointHook::~BreakpointHook() {
	this->Disable();

	auto it = std::find(s_hookList.begin(), s_hookList.end(), this);

	if (it != s_hookList.end()) {
        s_hookList.erase(it);
    }
}

long WINAPI BreakpointHook::onException(PEXCEPTION_POINTERS info) {
	for (auto it = s_hookList.begin(); it != s_hookList.end(); it++) {
		BreakpointHook* bp = *it;

		if (bp->m_addr == reinterpret_cast<std::uintptr_t>(info->ExceptionRecord->ExceptionAddress)) {
			bp->Disable();
			info->ContextRecord->EFlags |= 0x100;

			return EXCEPTION_CONTINUE_EXECUTION;
		} else if (info->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) {
			bp->Enable();
			info->ContextRecord->EFlags &= ~0x00000100; // Remove TRACE from EFLAGS

			bp->m_handler(info);

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	return EXCEPTION_CONTINUE_EXECUTION;
}
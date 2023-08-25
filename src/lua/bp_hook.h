#include <Windows.h>
#include <cstdint>
#include <functional>
#include <vector>

// Thanks to yeeeeeeee. for bp_hook code

class BreakpointHook {
public:
	BreakpointHook(std::uintptr_t addr, std::function<void(PEXCEPTION_POINTERS)> handler);
	void Enable();
	void Disable();
	~BreakpointHook();

	std::function<void(PEXCEPTION_POINTERS)> m_handler;

private:
	std::uintptr_t m_addr;
	std::uint8_t m_originalBytes;
	DWORD m_originalProtection;

	static long __stdcall onException(PEXCEPTION_POINTERS info);
};
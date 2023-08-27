#include <Windows.h>
#include <cstdint>
#include <functional>
#include <vector>
#include <sol.hpp>

// Thanks to yeeeeeeee. for bp_hook code

class BreakpointHook {
public:
	BreakpointHook(std::uintptr_t addr, sol::function callback, std::function<void(PEXCEPTION_POINTERS, sol::function)> handler);
	void Enable();
	void Disable();
	~BreakpointHook();

private:
	std::uintptr_t m_addr;
	std::uint8_t m_originalBytes;
	DWORD m_originalProtection;
	sol::function m_callback;
	std::function<void(PEXCEPTION_POINTERS, sol::function)> m_handler;

	static long __stdcall onException(PEXCEPTION_POINTERS info);
};
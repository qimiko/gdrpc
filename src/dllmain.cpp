#include "pch.h"
#include "gameLib.h"

void setupConsole() {
#ifdef _DEBUG
	AllocConsole();
	SetConsoleTitleA("generic name");
	FILE *fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);
#endif
}

bool hookDone = false;

// this function is included to allow adding dll to import address table
// it is never called
__declspec(dllexport) void __stdcall bean() {
	std::cout << "strawbby";
}

BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                    )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		DisableThreadLibraryCalls(hModule);
#ifdef _DEBUG
		setupConsole();
#endif
		if (!hookDone) {
			hookDone = true;
			doTheHook();
		}
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
        break;
  }
  return TRUE;
}


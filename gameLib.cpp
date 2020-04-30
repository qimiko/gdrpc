#include "pch.h"
#include "gameLib.h"

bool setupDone = false;

typedef int(__thiscall *MenuLayerInitF)(void * menuLayer);
MenuLayerInitF mli;

typedef void*(__fastcall *PlayLayerCreateF)(int * gameLevel);
PlayLayerCreateF plc;

typedef void(__fastcall *PlayLayerOnQuitF)(void * playLayer);
PlayLayerOnQuitF ploq;

typedef void*(__thiscall *PlayLayerShowNewBestF)(void *playLayer, char, float, int, char, char, char);
PlayLayerShowNewBestF plsnb;

typedef void(__thiscall *EditorPauseLayerOnExitEditorF)(void *editorPauseLayer, void*);
EditorPauseLayerOnExitEditorF eploee;

typedef void(__fastcall *LevelEditorLayerCreateF)(int *gameLevel);
LevelEditorLayerCreateF lelc;

typedef void(__thiscall *CCDirectorEndF)(void *CCDirector);
CCDirectorEndF ccde;

// this handles x button close
LONG_PTR oWindowProc;
LRESULT CALLBACK nWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
		// properly shutdown
		// idk what the correct function for this is tbh
		safeClose();
		break;
	}
	return CallWindowProc((WNDPROC)oWindowProc, hwnd, msg, wparam, lparam);
}

HMODULE GetCurrentModule()
{
	HMODULE hModule = NULL;
	GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			(LPCTSTR)GetCurrentModule,
			&hModule);

	return hModule;
}

int __fastcall MenuLayerInitH(void *menuLayer)
{
	// i have to test like this (idk why)
	if (!setupDone) {
		std::cout << "Running setup!";
		CreateThread(NULL, 0, mainThread, GetCurrentModule(), 0, NULL);
		setupDone = true;
	}
	std::cout << "MenuLayer::init" << std::endl;
	return mli(menuLayer);
}

void * __fastcall PlayLayerCreateH(int * gameLevel) {
#ifdef _DEBUG
	int maybeLevelID = *(int*)((int)gameLevel + 0xF8);
	std::stringstream ss;
	ss << "PlayLayer::create - " << maybeLevelID << " {" << std::hex << "0x" << (int)gameLevel << "}";

	std::cout << ss.str() << std::endl;
	SetConsoleTitleA(ss.str().c_str());
#endif

	currentGameLevel = gameLevel;

	currentPlayerState = playerState::level;
	updateTimestamp = true;
	updatePresence = true;

	return plc(gameLevel);
}

void __fastcall PlayLayerOnQuitH(void *playLayer) {
	std::cout << "PlayLayer::onQuit" << std::endl;
	SetConsoleTitleA("PlayLayer::onQuit");
	currentPlayerState = playerState::menu;
	updateTimestamp = true;
	updatePresence = true;
	return ploq(playLayer);
}

// to heck with your calling conventions
void * __fastcall PlayLayerShowNewBestH(void *playLayer, void * _edx, char p1, float p2, int p3, char p4, char p5, char p6)
{
#ifdef _DEBUG
	int levelID = *(int *)((int)currentGameLevel + 0xF8);
	int maybeBest = *(int *)((int)currentGameLevel + 0x248);
	std::stringstream ss;
	ss << "PlayLayer::showNewBest - " << levelID << " (" << maybeBest << "%)";
	std::cout << ss.str() << std::endl;
	SetConsoleTitleA(ss.str().c_str()); // i still cannot do std::cout for some really weird reason
#endif
	updatePresence = true;

	return plsnb(playLayer, p1, p2, p3, p4, p5, p6);
}

// thanks blaze for the other argument
void __fastcall EditorPauseLayerOnExitEditorH(void *editorPauseLayer, void* _edx, void* p1)
{
	std::cout << "EditorPauseLayer::onExitEditor" << std::endl;
	SetConsoleTitleA("EditorPauseLayer::onExitEditor");
	currentPlayerState = playerState::menu;
	updateTimestamp = true;
	updatePresence = true;

	return eploee(editorPauseLayer, p1);
}

void __fastcall LevelEditorLayerCreateH(int * gameLevel)
{
	std::cout << "LevelEditorLayer::create" << std::endl;
	SetConsoleTitleA("LevelEditorLayer::create");
	currentPlayerState = playerState::editor;
	updateTimestamp = true;
	updatePresence = true;

	currentGameLevel = gameLevel;

	lelc(gameLevel);
}

void __fastcall CCDirectorEndH(void *CCDirector)
{
	std::cout << "CCDirector::End";
	safeClose();
	ccde(CCDirector);
}

void doTheHook()
{
	if (MH_Initialize() != MH_OK)
	{
		MessageBoxA(0, "Hook init error!", "Error", MB_OK);
		return;
	}
	// menu layer test function thingy
	int* mladdr = (int*)((int)GetModuleHandleA("GeometryDash.exe") + 0x1907B0);
	MH_CreateHook(mladdr, reinterpret_cast<LPVOID*>(&MenuLayerInitH), reinterpret_cast<LPVOID*>(&mli));
	if (MH_EnableHook(mladdr) != MH_OK)
	{
		MessageBoxA(0, "MenuLayer::init hook error!", "Error", MB_OK);
	}

	// still a mess, i might redo this one day but i doubt it
	int *plcaddr = (int *)((int)GetModuleHandleA("GeometryDash.exe") + 0x1FB6D0);
	MH_CreateHook(plcaddr, reinterpret_cast<LPVOID *>(&PlayLayerCreateH), reinterpret_cast<LPVOID *>(&plc));
	if (MH_EnableHook(plcaddr) != MH_OK)
	{
		MessageBoxA(0, "PlayLayer::create hook error!", "Error", MB_OK);
	}

	int *ploqaddr = (int *)((int)GetModuleHandleA("GeometryDash.exe") + 0x20D810);
	MH_CreateHook(ploqaddr, reinterpret_cast<LPVOID *>(&PlayLayerOnQuitH), reinterpret_cast<LPVOID *>(&ploq));
	if (MH_EnableHook(ploqaddr) != MH_OK)
	{
		MessageBoxA(0, "PlayLayer::onQuit hook error!", "Error", MB_OK);
	}

	int *plsnbaddr = (int *)((int)GetModuleHandleA("GeometryDash.exe") + 0x1FE3A0);
	MH_CreateHook(plsnbaddr, reinterpret_cast<LPVOID *>(&PlayLayerShowNewBestH), reinterpret_cast<LPVOID *>(&plsnb));
	if (MH_EnableHook(plsnbaddr) != MH_OK)
	{
		MessageBoxA(0, "PlayLayer::showNewBest hook error!", "Error", MB_OK);
	}

	int *eploeeaddr = (int *)((int)GetModuleHandleA("GeometryDash.exe") + 0x75660);
	MH_CreateHook(eploeeaddr, reinterpret_cast<LPVOID *>(&EditorPauseLayerOnExitEditorH), reinterpret_cast<LPVOID *>(&eploee));
	if (MH_EnableHook(eploeeaddr) != MH_OK)
	{
		MessageBoxA(0, "EditorPauseLayer::onExitEditor hook error!", "Error", MB_OK);
	}

	int *lelcaddr = (int *)((int)GetModuleHandleA("GeometryDash.exe") + 0x76270);
	MH_CreateHook(lelcaddr, reinterpret_cast<LPVOID *>(&LevelEditorLayerCreateH), reinterpret_cast<LPVOID *>(&lelc));
	if (MH_EnableHook(lelcaddr) != MH_OK)
	{
		MessageBoxA(0, "LevelEditorLayer::create hook error!", "Error", MB_OK);
	}

	// setup closes
	oWindowProc = SetWindowLongPtrA(GetForegroundWindow(), GWL_WNDPROC, (LONG_PTR)nWindowProc);

	// close button calls this, x button calls wndproc
	HMODULE hMod = LoadLibrary(L"libcocos2d.dll");
	if (!hMod)
	{
		MessageBox(0, L"Failed to get libcocos!", L"Error", MB_OK);
	}
	int *ccdeaddr = (int *)GetProcAddress(hMod, "?end@CCDirector@cocos2d@@QAEXXZ");
	MH_CreateHook(ccdeaddr, reinterpret_cast<LPVOID *>(&CCDirectorEndH), reinterpret_cast<LPVOID *>(&ccde));
	if (MH_EnableHook(ccdeaddr) != MH_OK)
	{
		MessageBoxA(0, "CCDirector::end hook error!", "Error", MB_OK);
	}
}


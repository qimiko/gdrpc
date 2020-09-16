#include "gameLib.hpp"
#include "pch.h"

bool setupDone = false;

// this handles x button close
LONG_PTR oWindowProc;
LRESULT CALLBACK nWindowProc(HWND hwnd, UINT msg, WPARAM wparam,
                            LPARAM lparam) {
  switch (msg) {
  case WM_CLOSE:
    // properly shutdown
    // idk what the correct function for this is tbh
    get_game_loop()->close();
    break;
  }
  return CallWindowProc((WNDPROC)oWindowProc, hwnd, msg, wparam, lparam);
}

HMODULE GetCurrentModule() {
  HMODULE hModule = NULL;
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    (LPCTSTR)GetCurrentModule, &hModule);

  return hModule;
}

typedef int(__thiscall *MenuLayerInitF)(void *menuLayer);
MenuLayerInitF mli;

int __fastcall MenuLayerInitH(void *menuLayer) {
  // i have to test like this (idk why)
  if (!setupDone) {
    std::cout << "Running setup!";
    CreateThread(NULL, 0, mainThread, GetCurrentModule(), 0, NULL);
    setupDone = true;
  }
  std::cout << "MenuLayer::init" << std::endl;
  return mli(menuLayer);
}

typedef void *(__fastcall *PlayLayerCreateF)(int *gameLevel);
PlayLayerCreateF plc;

void *__fastcall PlayLayerCreateH(int *gameLevel) {
#ifdef _DEBUG
  int maybeLevelID = *(int *)((int)gameLevel + 0xF8);
  std::stringstream ss;
  ss << "PlayLayer::create - " << maybeLevelID << " {" << std::hex << "0x"
    << (int)gameLevel << "}";

  std::cout << ss.str() << std::endl;
  SetConsoleTitleA(ss.str().c_str());
#endif
  Game_Loop *game_loop = get_game_loop();

  if (game_loop->get_state() != playerState::editor || game_loop->get_reset_timestamp()) {
    game_loop->set_update_timestamp(true);
  }

  game_loop->set_state(playerState::level);
  game_loop->set_update_presence(true);
  game_loop->set_gamelevel(gameLevel);

  return plc(gameLevel);
}

typedef void(__fastcall *PlayLayerOnQuitF)(void *playLayer);
PlayLayerOnQuitF ploq;

void __fastcall PlayLayerOnQuitH(void *playLayer) {
  std::cout << "PlayLayer::onQuit" << std::endl;
  SetConsoleTitleA("PlayLayer::onQuit");

  Game_Loop *game_loop = get_game_loop();
  game_loop->set_state(playerState::menu);
  game_loop->set_update_timestamp(true);
  game_loop->set_update_presence(true);

  return ploq(playLayer);
}

typedef void *(__thiscall *PlayLayerShowNewBestF)(void *playLayer, char, float,
                                                  int, char, char, char);
PlayLayerShowNewBestF plsnb;

// to heck with your calling conventions
void *__fastcall PlayLayerShowNewBestH(void *playLayer, void *_edx, char p1,
                                      float p2, int p3, char p4, char p5,
                                      char p6) {
#ifdef _DEBUG

  int *current_gamelevel = get_game_loop()->get_gamelevel();

  int levelID = *(int *)((int)current_gamelevel + 0xF8);
  int maybeBest = *(int *)((int)current_gamelevel + 0x248);
  std::stringstream ss;
  ss << "PlayLayer::showNewBest - " << levelID << " (" << maybeBest << "%)";
  std::cout << ss.str() << std::endl;
  SetConsoleTitleA(
      ss.str()
          .c_str()); // i still cannot do std::cout for some really weird reason
#endif
  get_game_loop()->set_update_presence(true);

  return plsnb(playLayer, p1, p2, p3, p4, p5, p6);
}

typedef void(__thiscall *EditorPauseLayerOnExitEditorF)(void *editorPauseLayer,
                                                        void *);
EditorPauseLayerOnExitEditorF eploee;

// thanks blaze for the other argument
void __fastcall EditorPauseLayerOnExitEditorH(void *editorPauseLayer,
                                              void *_edx, void *p1) {

  std::cout << "EditorPauseLayer::onExitEditor" << std::endl;
  SetConsoleTitleA("EditorPauseLayer::onExitEditor");

  Game_Loop *game_loop = get_game_loop();
  game_loop->set_state(playerState::menu);
  game_loop->set_update_timestamp(true);
  game_loop->set_update_presence(true);

  return eploee(editorPauseLayer, p1);
}

typedef void *(__fastcall *LevelEditorLayerCreateF)(int *gameLevel);
LevelEditorLayerCreateF lelc;

void *__fastcall LevelEditorLayerCreateH(int *gameLevel) {
#ifdef _DEBUG
  std::stringstream ss;
  ss << "LevelEditorLayer::create - {" << std::hex << "0x" << (int)gameLevel
    << "}";
  std::cout << ss.str() << std::endl;
  SetConsoleTitleA(ss.str().c_str());
#endif

  Game_Loop * game_loop = get_game_loop();

  if (game_loop->get_state() != playerState::level || game_loop->get_reset_timestamp()) {
    game_loop->set_update_timestamp(true);
  }
  game_loop->set_state(playerState::editor);
  game_loop->set_update_presence(true);

  game_loop->set_gamelevel(gameLevel);

  return lelc(gameLevel);
}

typedef void(__thiscall *CCDirectorEndF)(void *CCDirector);
CCDirectorEndF ccde;

void __fastcall CCDirectorEndH(void *CCDirector) {
  std::cout << "CCDirector::End";
  get_game_loop()->close();
  ccde(CCDirector);
}

// no need to export this, not putting in .h
struct game_hook {
  int * orig_addr;
  void * hook_fn;
  void ** orig_fn;
  std::string fn_name;
};

void doTheHook() {
  if (MH_Initialize() != MH_OK) {
    MessageBoxA(0, "Hook init error!", "Error", MB_OK);
    return;
  }

  HMODULE gd_handle = GetModuleHandleA("GeometryDash.exe");
  HMODULE cocos_handle = LoadLibrary(L"libcocos2d.dll");

  // close button calls this, x button calls wndproc
  if (!cocos_handle) {
    MessageBox(0, L"Failed to get libcocos!", L"Error", MB_OK);
  }

  // setup closes
  oWindowProc = SetWindowLongPtrA(GetForegroundWindow(), GWL_WNDPROC,
                                  (LONG_PTR)nWindowProc);

  // wall of hooks
  std::vector<game_hook> hooks{
      {(int *)((int)gd_handle + 0x1907B0),
       reinterpret_cast<void *>(&MenuLayerInitH),
       reinterpret_cast<void **>(&mli), "MenuLayer::init"},
      {(int *)((int)gd_handle + 0x1FB6D0),
       reinterpret_cast<void *>(&PlayLayerCreateH),
       reinterpret_cast<void **>(&plc), "PlayLayer::create"},
      {(int *)((int)gd_handle + 0x20D810),
       reinterpret_cast<void *>(&PlayLayerOnQuitH),
       reinterpret_cast<void **>(&ploq), "PlayLayer::onQuit"},
      {(int *)((int)gd_handle + 0x1FE3A0),
       reinterpret_cast<void *>(&PlayLayerShowNewBestH),
       reinterpret_cast<void **>(&plsnb), "PlayLayer::showNewBest"},
      {(int *)((int)gd_handle + 0x75660),
       reinterpret_cast<void *>(&EditorPauseLayerOnExitEditorH),
       reinterpret_cast<void **>(&eploee), "EditorPauseLayer::onExitEditor"},
      {(int *)((int)gd_handle + 0x76270),
       reinterpret_cast<void *>(&LevelEditorLayerCreateH),
       reinterpret_cast<void **>(&lelc), "LevelEditorLayer::create"},
      {(int *)GetProcAddress(cocos_handle, "?end@CCDirector@cocos2d@@QAEXXZ"),
       reinterpret_cast<void *>(&CCDirectorEndH),
       reinterpret_cast<void **>(&ccde), "CCDirector::end"},
  };

  for (const auto& hook: hooks) {
    MH_CreateHook(hook.orig_addr, hook.hook_fn, hook.orig_fn);
    if (MH_EnableHook(hook.orig_addr) != MH_OK) {
      std::stringstream ss;
      ss << hook.fn_name << " hook error!"; // inefficient tbh
      MessageBoxA(0, ss.str().c_str(), "Error", MB_OK);
    }
  }
}
#include "game_hooks.hpp"
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
  if (!setupDone) {
    // logger isn't initialized at this point, all calls go to console
    get_game_loop()->register_on_initialize([] {
      // now we can log
      auto logger = get_game_loop()->get_logger();

      logger->info(FMT_STRING("initialized:\n\
GD Handle: {:#x}\n\
MenuLayer::init: {:#x}"),
                  (int)GetModuleHandleA("GeometryDash.exe"),
                  reinterpret_cast<int>(&mli));
    });

    CreateThread(NULL, 0, mainThread, GetCurrentModule(), 0, NULL);
    setupDone = true;
  }
  return mli(menuLayer);
}

typedef void *(__fastcall *PlayLayerCreateF)(int *gameLevel);
PlayLayerCreateF plc;

void *__fastcall PlayLayerCreateH(int *gameLevel) {
  int levelID = *(int *)((int)gameLevel + 0xF8);

  Game_Loop *game_loop = get_game_loop();

// funny conversion because fmt likes to just not send the message
  game_loop->get_logger()->debug(FMT_STRING("PlayLayer::create ({:#x}) called:\n\
levelID: {} @ {:#x}"),
    reinterpret_cast<int>(&plc),
    levelID,
    reinterpret_cast<int>(gameLevel));

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
  Game_Loop *game_loop = get_game_loop();

  game_loop->get_logger()->debug(FMT_STRING("PlayLayer::onQuit ({:#x}) called"),
    reinterpret_cast<int>(&ploq));
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
  Game_Loop *game_loop = get_game_loop();

  int *current_gamelevel = game_loop->get_gamelevel();
  int levelID = *(int *)((int)current_gamelevel + 0xF8);
  int new_best = *(int *)((int)current_gamelevel + 0x248);

  game_loop->get_logger()->debug(FMT_STRING("PlayLayer::showNewBest ({:#x}) called\n\
levelID: {}, got {}%"),
    reinterpret_cast<int>(&plsnb),
    levelID,
    new_best);

  game_loop->set_update_presence(true);

  return plsnb(playLayer, p1, p2, p3, p4, p5, p6);
}

typedef void(__thiscall *EditorPauseLayerOnExitEditorF)(void *editorPauseLayer,
                                                        void *);
EditorPauseLayerOnExitEditorF eploee;

// thanks blaze for the other argument
void __fastcall EditorPauseLayerOnExitEditorH(void *editorPauseLayer,
                                              void *_edx, void *p1) {

  Game_Loop *game_loop = get_game_loop();

  game_loop->get_logger()->debug(
      FMT_STRING("EditorPauseLayer::onExitEditor ({:#x}) called"),
      reinterpret_cast<int>(&eploee));

  game_loop->set_state(playerState::menu);
  game_loop->set_update_timestamp(true);
  game_loop->set_update_presence(true);

  return eploee(editorPauseLayer, p1);
}

typedef void *(__fastcall *LevelEditorLayerCreateF)(int *gameLevel);
LevelEditorLayerCreateF lelc;

void *__fastcall LevelEditorLayerCreateH(int *gameLevel) {
  Game_Loop * game_loop = get_game_loop();

  game_loop->get_logger()->debug("LevelEditorLayer::create ({:#x}) called",
    reinterpret_cast<int>(&lelc));

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
  Game_Loop *game_loop = get_game_loop();
  game_loop->get_logger()->debug("CCDirector::end ({:#x}) called",
    reinterpret_cast<int>(&ccde));
  game_loop->close();

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
  std::array<game_hook, 7> hooks { {
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
  } };

  for (const auto& hook: hooks) {
    MH_CreateHook(hook.orig_addr, hook.hook_fn, hook.orig_fn);
    if (MH_EnableHook(hook.orig_addr) != MH_OK) {
      auto s = fmt::format(FMT_STRING("error hooking function {}"), hook.fn_name);
      MessageBoxA(0, s.c_str(), "gdrpc error", MB_OK);
    }
  }
}
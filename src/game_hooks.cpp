#include "game_hooks.hpp"

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

template <typename T> T *offset_from_base(void *struct_ptr, int addr) {
  return reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(struct_ptr) + addr);
}

int(__thiscall *MenuLayer_init_O)(void *menuLayer);
int __fastcall MenuLayer_init_H(void *menuLayer) {
  if (!setupDone) {
    if (auto logger = get_game_loop()->get_logger()) {
      logger->trace(FMT_STRING("menu layer setup called"));
    }

    CreateThread(NULL, 0, mainThread, GetCurrentModule(), 0, NULL);
    setupDone = true;
  }
  return MenuLayer_init_O(menuLayer);
}

void *(__fastcall *PlayLayer_create_O)(GJGameLevel *gameLevel);
void *__fastcall PlayLayer_create_H(GJGameLevel *gameLevel) {
  int levelID = gameLevel->levelID;

  Game_Loop *game_loop = get_game_loop();

  if (auto logger = game_loop->get_logger()) {
    logger->debug(FMT_STRING("PlayLayer::create called:\n\
levelID: {} @ {:#x}"),
                  levelID, reinterpret_cast<int>(gameLevel));
  }

  if (game_loop->get_state() != playerState::editor ||
      game_loop->get_reset_timestamp(gameLevel->levelFolder)) {
    game_loop->set_update_timestamp(true);
  }

  game_loop->set_state(playerState::level);
  game_loop->set_update_presence(true);
  game_loop->set_gamelevel(gameLevel);

  return PlayLayer_create_O(gameLevel);
}

void(__fastcall *PlayLayer_onQuit_O)(void *playLayer);
void __fastcall PlayLayer_onQuit_H(void *playLayer) {
  Game_Loop *game_loop = get_game_loop();

  if (auto logger = game_loop->get_logger()) {
    logger->debug(FMT_STRING("PlayLayer::onQuit called"));
  }

  game_loop->set_state(playerState::menu);
  game_loop->set_update_timestamp(true);
  game_loop->set_update_presence(true);

  return PlayLayer_onQuit_O(playLayer);
}

void *(__thiscall *PlayLayer_showNewBest_O)(void *playLayer, char, float, int,
                                            char, char, char);
void *__fastcall PlayLayer_showNewBest_H(void *playLayer, void *_edx, char p1,
                                         float p2, int p3, char p4, char p5,
                                         char p6) {

  Game_Loop *game_loop = get_game_loop();

  auto current_gamelevel = game_loop->get_gamelevel();
  int levelID = current_gamelevel->levelID;
  int new_best = current_gamelevel->normalPercent;

  if (auto logger = game_loop->get_logger()) {
    logger->debug(FMT_STRING("PlayLayer::showNewBest called\n\
levelID: {}, got {}%"),
                  levelID, new_best);
  }

  game_loop->set_update_presence(true);

  return PlayLayer_showNewBest_O(playLayer, p1, p2, p3, p4, p5, p6);
}

// thanks blaze for the other argument
void(__thiscall *EditorPauseLayer_onExitEditor_O)(void *editorPauseLayer,
                                                  void *);
void __fastcall EditorPauseLayer_onExitEditor_H(void *editorPauseLayer,
                                                void *_edx, void *p1) {
  Game_Loop *game_loop = get_game_loop();

  if (auto logger = game_loop->get_logger()) {
    logger->debug(FMT_STRING("EditorPauseLayer::onExitEditor called"));
  }

  game_loop->set_state(playerState::menu);
  game_loop->set_update_timestamp(true);
  game_loop->set_update_presence(true);

  return EditorPauseLayer_onExitEditor_O(editorPauseLayer, p1);
}

void *(__fastcall *LevelEditorLayer_create_O)(GJGameLevel *gameLevel);
void *__fastcall LevelEditorLayer_create_H(GJGameLevel *gameLevel) {
  Game_Loop *game_loop = get_game_loop();

  int levelID = gameLevel->levelID;

  if (auto logger = game_loop->get_logger()) {
    logger->debug(FMT_STRING("LevelEditorLayer::create called:\n\
levelID: {} @ {:#x}"),
                  levelID, reinterpret_cast<int>(gameLevel));
  }
  if (game_loop->get_state() != playerState::level ||
      game_loop->get_reset_timestamp()) {
    game_loop->set_update_timestamp(true);
  }
  game_loop->set_state(playerState::editor);
  game_loop->set_update_presence(true);

  game_loop->set_gamelevel(gameLevel);

  return LevelEditorLayer_create_O(gameLevel);
}

void fix_object_count(void *LevelEditorLayer, GJGameLevel *level) {
  const auto LevelEditorLayer_objectCount_offset = 0x3A0;
  const auto size_of_int = sizeof(int);

  level->objectCount_rand = *offset_from_base<int>(
      LevelEditorLayer,
      LevelEditorLayer_objectCount_offset - (size_of_int * 2));
  level->objectCount_seed = *offset_from_base<int>(
      LevelEditorLayer, LevelEditorLayer_objectCount_offset - size_of_int);
  level->objectCount = *offset_from_base<int>(
      LevelEditorLayer, LevelEditorLayer_objectCount_offset);
}

void(__thiscall *LevelEditorLayer_addSpecial_O)(void *LevelEditorLayer,
                                                void *object);

void __fastcall LevelEditorLayer_addSpecial_H(void *self, void *_edx,
                                              void *object) {
  LevelEditorLayer_addSpecial_O(self, object);

  Game_Loop *game_loop = get_game_loop();
  auto gamelevel = game_loop->get_gamelevel();

  // this should hopefully prevent spam upon level loading
  auto new_object_count = *offset_from_base<int>(self, 0x3A0);
  if (gamelevel->objectCount >= new_object_count)
    return;

  fix_object_count(self, gamelevel);
  game_loop->set_update_presence(true);

  if (auto logger = game_loop->get_logger()) {
    logger->debug(
        FMT_STRING("LevelEditorLayer::addSpecial called\nnew obj count: {}"),
        gamelevel->objectCount);
  }

  return;
}

void(__thiscall *LevelEditorLayer_removeSpecial_O)(void *LevelEditorLayer,
                                                   void *object);
void __fastcall LevelEditorLayer_removeSpecial_H(void *self, void *_edx,
                                                 void *object) {
  LevelEditorLayer_removeSpecial_O(self, object);

  Game_Loop *game_loop = get_game_loop();
  auto gamelevel = game_loop->get_gamelevel();

  // same thing but with level closing
  auto new_object_count = *offset_from_base<int>(self, 0x3A0);
  if (gamelevel->objectCount < new_object_count)
    return;

  fix_object_count(self, gamelevel);
  game_loop->set_update_presence(true);

  if (auto logger = game_loop->get_logger()) {
    logger->debug(
        FMT_STRING("LevelEditorLayer::removeSpecial called\nnew obj count: {}"),
        gamelevel->objectCount);
  }

  return;
}

void(__thiscall *CCDirector_end_O)(void *CCDirector);
void __fastcall CCDirector_end_H(void *CCDirector) {
  Game_Loop *game_loop = get_game_loop();

  if (auto logger = game_loop->get_logger()) {
    logger->debug(FMT_STRING("CCDirector::end called"));
  }

  game_loop->close();

  CCDirector_end_O(CCDirector);
}

// no need to export this, not putting in .h
struct game_hook {
  void *orig_addr;
  void *hook_fn;
  void **orig_fn;
};

#define CREATE_HOOK(ADDRESS, NAME)                                             \
  {                                                                            \
    reinterpret_cast<void *>(ADDRESS), reinterpret_cast<void *>(&(NAME##_H)),  \
        reinterpret_cast<void **>(&(NAME##_O))                                 \
  }

#define CREATE_GD_HOOK(ADDRESS, NAME)                                          \
  CREATE_HOOK(offset_from_base<void>(gd_handle, ADDRESS), NAME)

void doTheHook() {
  Game_Loop *game_loop = get_game_loop();

  try {
    game_loop->initialize_config();
  } catch (const std::exception &e) {
    auto message = fmt::format(
        FMT_STRING("Initialization of config failed with\n{}."), e.what());
    game_loop->display_error(message);
    return;
  }

  auto logger = game_loop->get_logger();

  if (auto status = MH_Initialize(); status != MH_OK) {
    auto message =
        fmt::format(FMT_STRING("Hook init error with code {}."), status);
    game_loop->display_error(message);

    return;
  }

  std::string gd_name = game_loop->get_executable_name();

  HMODULE gd_handle = GetModuleHandleA(gd_name.c_str());
  HMODULE cocos_handle = LoadLibraryA("libcocos2d.dll");

  // close button calls this, x button calls wndproc
  if (!gd_handle) {
    auto message = fmt::format(
        FMT_STRING("Failed to get Geometry Dash using name {}"), gd_name);
    game_loop->display_error(message);
    return;
  }

  if (logger) {
    logger->trace(FMT_STRING("found {} at {:#x}"), gd_name, (int)gd_handle);
  }

  if (!cocos_handle) {
    game_loop->display_error("Failed to open a handle to libcocos!");
    return;
  }

  // setup closes
  oWindowProc = SetWindowLongPtrA(GetForegroundWindow(), GWL_WNDPROC,
                                  (LONG_PTR)nWindowProc);

  // wall of hooks
  std::array<game_hook, 9> hooks{{
      CREATE_GD_HOOK(0x1907B0, MenuLayer_init),
      CREATE_GD_HOOK(0x1FB6D0, PlayLayer_create),
      CREATE_GD_HOOK(0x20D810, PlayLayer_onQuit),
      CREATE_GD_HOOK(0x1FE3A0, PlayLayer_showNewBest),
      CREATE_GD_HOOK(0x75660, EditorPauseLayer_onExitEditor),
      CREATE_GD_HOOK(0x15ED60, LevelEditorLayer_create),
      CREATE_GD_HOOK(0x162650, LevelEditorLayer_addSpecial),
      CREATE_GD_HOOK(0x162FF0, LevelEditorLayer_removeSpecial),
      CREATE_HOOK(
        GetProcAddress(cocos_handle, "?end@CCDirector@cocos2d@@QAEXXZ"),
        CCDirector_end)
  }};

  for (const auto &hook : hooks) {
    MH_CreateHook(hook.orig_addr, hook.hook_fn, hook.orig_fn);
    if (auto status = MH_EnableHook(hook.orig_addr); status != MH_OK) {
      auto message = fmt::format(
          FMT_STRING("error hooking function with code {}"), status);
      game_loop->display_error(message);
    }
  }

  if (logger) {
    logger->debug("hooks setup");
  }
}
#pragma once
#include "config_defaults.hpp"
#include "gdapi.hpp"
#include "gjgamelevel.hpp"
#include "pch.h"
#include "presence_wrapper.hpp"

#include <algorithm>
#include <ctime>
#include <functional>
#include <vector>

#include <fstream>
#include <toml.hpp>

#include <fmt/format.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#ifndef RICHPRESENCEUTIL_H
#define RICHPRESENCEUTIL_H
enum class playerState {
  level,
  editor,
  menu,
};

class Game_Loop {
private:
  playerState player_state;
  GJGameLevel *gamelevel;
  GDlevel level;

  std::shared_ptr<spdlog::logger> logger;

  bool update_presence, update_timestamp;

  time_t current_timestamp;
  Discord_Presence *discord;

  Config::Config_Format config;

  std::string large_text;

  // wrapper to presence update that allows strings + debug messages
  void update_presence_w(std::string &, std::string &, std::string &,
                         std::string &, std::string &);

  std::function<void(Game_Loop *)> on_initialize;

public:
  Game_Loop();

  playerState get_state();
  void set_state(playerState);

  GJGameLevel *get_gamelevel();
  void set_gamelevel(GJGameLevel *);

  void set_update_presence(bool);
  void set_update_timestamp(bool);

  bool get_reset_timestamp(int folder = 0);

  std::string get_executable_name();

  std::shared_ptr<spdlog::logger> get_logger();

  void on_loop();

  void initialize_config();
  void initialize_loop();

  void close();

  void register_on_initialize(std::function<void(Game_Loop *)>);

  void display_error(std::string message);
};

DWORD WINAPI mainThread(LPVOID lpParam);

Game_Loop *get_game_loop();
#endif
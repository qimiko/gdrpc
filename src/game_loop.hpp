#pragma once
#include "gdapi.hpp"
#include "pch.h"
#include "presence_wrapper.hpp"
#include <algorithm>
#include <ctime>
#include <functional>
#include <stdio.h>

#include <fstream>
#include <toml.hpp>

#include <fmt/format.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#ifndef RICHPRESENCEUTIL_H
#define RICHPRESENCEUTIL_H
enum class playerState {
  level,
  editor,
  menu,
};

struct configPresence {
  std::string detail;
  std::string state;
  std::string smalltext;

  void from_toml(const toml::value &table) {
    this->detail = toml::find<std::string>(table, "detail");
    this->state = toml::find<std::string>(table, "state");
    this->smalltext = toml::find<std::string>(table, "smalltext");
  }

  toml::table into_toml() const {
    return toml::table{{"detail", this->detail},
                       {"state", this->state},
                       {"smalltext", this->smalltext}};
  }
};

class Game_Loop {
private:
  playerState player_state;
  int *gamelevel;
  GDlevel level;

  std::shared_ptr<spdlog::logger> logger;

  bool update_presence;
  bool update_timestamp;

  time_t current_timestamp;
  Discord_Presence *discord;

  configPresence saved_level, playtesting_level, error_level, editor_status,
      menu_status;
  bool editor_reset_timestamp;
  bool output_logging;

  // wrapper to presence update that allows strings + debug messages
  void update_presence_w(std::string &, std::string &, std::string &,
                         std::string &, std::string &);

  std::string large_text;

  std::function<void()> on_initialize;

public:
  Game_Loop();

  playerState get_state();
  void set_state(playerState);

  int *get_gamelevel();
  void set_gamelevel(int *);

  void set_update_presence(bool);
  void set_update_timestamp(bool);

  bool get_reset_timestamp();

  std::shared_ptr<spdlog::logger> get_logger();

  void on_loop();
  void initialize();

  void close();

  void log();

  void register_on_initialize(std::function<void()>);
};

DWORD WINAPI mainThread(LPVOID lpParam);

Game_Loop *get_game_loop();
#endif
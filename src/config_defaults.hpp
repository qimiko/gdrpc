#pragma once
#ifndef CONFIG_DEFAULTS_HPP
#define CONFIG_DEFAULTS_HPP

#include <toml.hpp>

namespace Config {

constexpr int LATEST_VERSION = 3;

constexpr auto DEFAULT_EXECUTABLE = "GeometryDash.exe";
constexpr auto DEFAULT_URL = "http://boomlings.com";
constexpr auto DEFAULT_PREFIX = "/database/";

struct Presence {
  std::string detail;
  std::string state;
  std::string smalltext;

  void from_toml(const toml::value &table) {
    this->detail = toml::find<std::string>(table, "detail");
    this->state = toml::find<std::string>(table, "state");
    this->smalltext = toml::find<std::string>(table, "smalltext");
  }

  toml::value into_toml() const {
    return toml::table{{"detail", this->detail},
                       {"state", this->state},
                       {"smalltext", this->smalltext}};
  }
};

struct Config_Format {
  struct Level {
    Config::Presence saved;
    Config::Presence playtesting;
    Config::Presence error;

    void from_toml(const toml::value &table) {
      this->saved = toml::find<Config::Presence>(table, "saved");
      this->playtesting = toml::find<Config::Presence>(table, "playtesting");
      this->error = toml::find<Config::Presence>(table, "error");
    }

    toml::value into_toml() const {
      return toml::table{{"saved", this->saved},
                         {"playtesting", this->playtesting},
                         {"error", this->error}};
    }
  };

  struct Editor : Config::Presence {
    bool reset_timestamp;

    void from_toml(const toml::value &table) {
      this->detail = toml::find<std::string>(table, "detail");
      this->state = toml::find<std::string>(table, "state");
      this->smalltext = toml::find<std::string>(table, "smalltext");
      this->reset_timestamp =
          toml::find_or<bool>(table, "reset_timestamp", reset_timestamp);
    }

    toml::value into_toml() const {
      return toml::table{{"detail", this->detail},
                         {"state", this->state},
                         {"smalltext", this->smalltext},
                         {"reset_timestamp", this->reset_timestamp}};
    }
  };

  struct User {
    std::string ranked;
    std::string default;
    bool get_rank;

    void from_toml(const toml::value &table) {
      this->ranked = toml::find<std::string>(table, "ranked");
      this->default = toml::find<std::string>(table, "default");
      this->get_rank = toml::find<bool>(table, "get_rank");
    }

    toml::value into_toml() const {
      return toml::table{{"ranked", this->ranked},
                         {"default", this->default},
                         {"get_rank", this->get_rank}};
    }
  };

  struct Settings {
    int file_version;
    bool logging;
    std::string executable_name;
    std::string base_url;
    std::string url_prefix;

    void from_toml(const toml::value &table) {
      this->file_version = toml::find<int>(table, "file_version");
      this->logging = toml::find<bool>(table, "logging");
      this->executable_name = toml::find_or<std::string>(
          table, "executable_name", DEFAULT_EXECUTABLE);
      this->base_url =
          toml::find_or<std::string>(table, "base_url", DEFAULT_URL);
      this->url_prefix =
          toml::find_or<std::string>(table, "url_prefix", DEFAULT_PREFIX);
    }

    toml::value into_toml() const {
      return toml::table{{"file_version", this->file_version},
                         {"logging", this->logging},
                         {"executable_name", this->executable_name},
                         {"base_url", this->base_url},
                         {"url_prefix", this->url_prefix}};
    }
  };

  void from_toml(const toml::value &table) {
    this->level = toml::find<Config_Format::Level>(table, "level");
    this->editor = toml::find<Config_Format::Editor>(table, "editor");
    this->user = toml::find<Config_Format::User>(table, "user");
    this->menu = toml::find<Config::Presence>(table, "menu");

    if (table.contains("settings")) {
      // this table is still optional due to previous versions not containing it
      this->settings = toml::find<Config_Format::Settings>(table, "settings");
    }
  }

  toml::value into_toml() const {
    return toml::table{{"level", this->level},
                       {"editor", this->editor},
                       {"user", this->user},
                       {"menu", this->menu},
                       {"settings", this->settings}};
  }

  Level level = {
      {"Playing {name}", "by {author} ({best}%)", "{stars}* {diff} ({id})"},
      {"Editing a level", "", ""},
      {"Playing a level", "", ""}};
  Editor editor = {{"Editing a level", "", ""}, false};
  User user = {"{name} [Rank #{rank}]", "", true};
  Config::Presence menu = {"Idle", "", ""};

  Settings settings = {Config::LATEST_VERSION, false,
                       Config::DEFAULT_EXECUTABLE, Config::DEFAULT_URL,
                       Config::DEFAULT_PREFIX};
};
} // namespace Config

#endif
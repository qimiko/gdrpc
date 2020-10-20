#include "game_loop.hpp"
#include "pch.h"

Game_Loop game_loop = Game_Loop();

int *get_address(int *start, std::vector<int> addresses) {
  // start with value of 1st address
  auto it = addresses.begin();
  int *cur_addr = (int *)((int)start + *it);
  std::advance(it, 1);

  while (it != addresses.end()) {
    // get next and increment all at once!
    cur_addr = (int *)(*cur_addr + *it);
    std::advance(it, 1); // null pointer protection
  }
  return cur_addr;
}

// insane_demon to Insane Demon
std::string getTextFromKey(std::string key) {
  key[0] = toupper(key[0]); // uppercase first letter
  int index = 1; // start from 1 because we start string from 1, and also
                 // because you can't get an index at -1 like it'll try to do..
  std::for_each(key.begin() + 1, key.end(), [&index, &key](char &letter) {
    // in any case, this checks if it's a space before
    if (key[index - 1] == ' ') {
      letter = toupper(letter); // then capitalizes
    } else if (letter == '_') { // if underscore it goes and remoes it
      letter = ' ';
    }
    index++; // then we use this to count
  });
  return key;
}

std::string formatWithLevel(std::string &s, GDlevel &level,
                            int currentBest = 0) {
  std::string f;
  try {
    f = fmt::format(s, fmt::arg("id", level.levelID),
                    fmt::arg("name", level.name), fmt::arg("best", currentBest),
                    fmt::arg("diff", getTextFromKey(getDifficultyName(level))),
                    fmt::arg("author", level.author),
                    fmt::arg("stars", level.stars));
  } catch (const fmt::format_error &e) {

    std::string error_string =
        fmt::format("error found while parsing {}\n{}", s, e.what());

    get_game_loop()->get_logger()->critical(error_string);
    MessageBoxA(0, error_string.c_str(), "formatter error", MB_OK);
    f = s;
  } catch (...) {
    get_game_loop()->get_logger()->critical(
        "unknown error found while trying to parse {}", s);
    MessageBoxA(0, "idk", "unknown format error", MB_OK);
    f = s;
  }
  return f;
}

Game_Loop *get_game_loop() { return &game_loop; }

void Game_Loop::update_presence_w(std::string &details, std::string &largeText,
                                  std::string &smallText, std::string &state,
                                  std::string &smallImage) {

  if (logger) {
    logger->info("setting presence:\n\
details: `{}` | state: `{}`\n\
small_text: `{}` | large_text: `{}`\n\
timestamp_update: {}",
                 details, state, smallText, largeText, update_timestamp);
  }

  if (update_timestamp) {
    current_timestamp = time(0);
    update_timestamp = false;
  }
  discord->update(details.c_str(), largeText.c_str(), smallText.c_str(),
                  state.c_str(), smallImage.c_str(), current_timestamp);
}

void Game_Loop::close() {
  if (logger) {
    logger->warn("shutdown called!");
  }
  discord->shutdown();
}

Game_Loop::Game_Loop()
    : player_state(playerState::menu), current_timestamp(time(0)),
      gamelevel(nullptr), update_presence(false), update_timestamp(false),
      editor_reset_timestamp(false), output_logging(false), get_rank(true),
      discord(get_discord()), logger(nullptr) {
  on_initialize = [] {
  }; // blank function so it doesn't complain about how i don't initialize
}

void Game_Loop::initialize_config() {
  // config time!

  // we fill in defaults to aid in creation
  saved_level = {"Playing {name}", "by {author} ({best}%)",
                 "{stars}* {diff} ({id})"};
  playtesting_level = {"Editing a level", "", ""};
  error_level = {"Playing a level", "", ""};
  editor_status = {"Editing a level", "", ""};
  menu_status = {"Idle", "", ""};

  user_ranked = "{name} [Rank #{rank}]";
  user_default = "";

  executable_name = "GeometryDash.exe";

  static const int latest_version = 3;
  int file_version = 1;

  try {
    const std::string filename = "gdrpc.toml";
    if (!std::ifstream(filename)) {
      // create generic file
      std::ofstream config_file(filename);

      const toml::value user{{"ranked", user_ranked},
                             {"default", user_default},
                             {"get_rank", get_rank}};

      const toml::value menu(menu_status);

      // can't use const here due to timestamp
      toml::value editor(editor_status);
      editor.as_table()["reset_timestamp"] = editor_reset_timestamp;

      const toml::value saved(saved_level);
      const toml::value playtesting(playtesting_level);
      const toml::value error(error_level);

      const toml::value settings{{"file_version", latest_version},
                                 {"logging", output_logging},
                                 {"executable_name", executable_name}};

      const toml::value level{
          {"saved", saved}, {"playtesting", playtesting}, {"error", error}};

      const toml::value default_config{{"level", level},
                                       {"editor", editor},
                                       {"menu", menu},
                                       {"user", user},
                                       {"settings", settings}};

      config_file << "# autogenerated config\n" << default_config << std::endl;
    }
    const toml::value config = toml::parse(filename);

    // setting up strings

    // level
    const auto level_table = toml::find(config, "level");

    saved_level = toml::find<configPresence>(level_table, "saved");
    playtesting_level = toml::find<configPresence>(level_table, "playtesting");
    error_level = toml::find<configPresence>(level_table, "error");

    // editor
    const auto editor_table = toml::find(config, "editor");
    editor_status = toml::find<configPresence>(config, "editor");
    editor_reset_timestamp = toml::find_or<bool>(
        editor_table, "reset_timestamp", editor_reset_timestamp);

    // menu
    menu_status = toml::find<configPresence>(config, "menu");

    // user (large text)
    const auto user_table = toml::find(config, "user");
    user_ranked = toml::find_or<std::string>(user_table, "ranked", user_ranked);
    user_default =
        toml::find_or<std::string>(user_table, "default", user_default);
    get_rank = toml::find_or<bool>(user_table, "get_rank", get_rank);

    // settings is a new key, don't expect people to have it
    if (config.contains("settings")) {
      const auto settings_table = toml::find(config, "settings");
      output_logging =
          toml::find_or<bool>(settings_table, "logging", output_logging);

      // we can already assume 2 as 2 has settings table
      file_version = toml::find_or<int>(settings_table, "file_version", 2);

      if (file_version >= 3) {
        executable_name = toml::find_or<std::string>(
            settings_table, "executable_name", executable_name);
      }
    }

  } catch (const std::exception &e) {
    if (logger) {
      logger->critical("error found while parsing config\n{}", e.what());
    }
    MessageBoxA(0, e.what(), "config parser error", MB_OK);
  } catch (...) {
    if (logger) {
      logger->critical("unknown config parsing error");
    }
    MessageBoxA(0, "unhandled error\r\nthings should continue fine",
                "config parser", MB_OK);
  }

  // on debug builds, console logging will be enabled
  if (output_logging) {
    logger = spdlog::basic_logger_mt("rpclog", "gdrpc.log", true);
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::debug);
    logger->info("gdrpc v{}", latest_version);
  }
}

void Game_Loop::initialize_loop() {
  discord->initialize();

  large_text = user_default;

  int *gd_base = (int *)GetModuleHandleA(executable_name.c_str());

  int *accountID = get_address(gd_base, {0x3222D8, 0x120});

  GD_Client client = GD_Client("boomlings.com");

  GDuser user;
  if (get_rank) {
    if (logger) {
      logger->debug("getting infomation for user {}", *accountID);
    }
    try {
      bool userInfoSuccess = client.get_user_info(*accountID, user);
      if (userInfoSuccess) {
        client.get_user_rank(user);
      }
    } catch (const std::exception &e) {
      if (logger) {
        logger->warn("failed to get user info or rank\n{}", e.what());
      }
    }
  }

  if (user.rank != -1) {
    large_text = fmt::format(user_ranked, fmt::arg("name", user.name),
                             fmt::arg("rank", user.rank));
  } else {
    char *username = (char *)(get_address(gd_base, {0x3222D8, 0x108}));
    large_text = std::string(username); // hopeful fallback
  }

  update_presence = true;
  update_timestamp = true;
  on_initialize();
}

void Game_Loop::on_loop() {
  discord->run_callbacks();
  if (update_presence) {
    // we don't need to keep this in the class, every time presence updates it
    // gets remade anyways
    std::string details, state, small_text, small_image;

    switch (player_state) {
    case playerState::level: {
      int level_location = *(int *)((int)gamelevel + 0x364);
      int current_best = *(int *)((int)gamelevel + 0x248);
      if (!parseGJGameLevel(gamelevel, level)) {
        if (logger) {
          logger->critical("failed to parse gamelevel at {#x}", (int)gamelevel);
        }
        details =
            fmt::format(error_level.detail, fmt::arg("best", current_best));
        state = fmt::format(error_level.state, fmt::arg("best", current_best));
        small_text = error_level.smalltext;
        small_image = "";
      } else if (level_location == 2) {
        details =
            formatWithLevel(playtesting_level.detail, level, current_best);
        state = formatWithLevel(playtesting_level.state, level, current_best);
        small_text =
            formatWithLevel(playtesting_level.smalltext, level, current_best);
        small_image = "creator_point";
      } else {
        details = formatWithLevel(saved_level.detail, level, current_best);
        state = formatWithLevel(saved_level.state, level, current_best);
        small_text =
            formatWithLevel(saved_level.smalltext, level, current_best);
        small_image = getDifficultyName(level);
      }
      break;
    }
    case playerState::editor: {
      details = editor_status.detail;
      state = editor_status.state;
      small_text = editor_status.smalltext;
      small_image = "creator_point";
      break;
    }
    case playerState::menu: {
      details = menu_status.detail;
      state = menu_status.state;
      small_text = menu_status.smalltext;
      small_image = "";
      break;
    }
    }
    update_presence_w(details, large_text, small_text, state, small_image);
    update_presence = false;
  }
}

void Game_Loop::set_update_presence(bool n_presence) {
  update_presence = n_presence;
}

void Game_Loop::set_update_timestamp(bool n_timestamp) {
  update_timestamp = n_timestamp;
}

void Game_Loop::set_gamelevel(int *n_gamelevel) { gamelevel = n_gamelevel; }

int *Game_Loop::get_gamelevel() { return gamelevel; }

void Game_Loop::set_state(playerState n_state) { player_state = n_state; }

playerState Game_Loop::get_state() { return player_state; }

bool Game_Loop::get_reset_timestamp() { return editor_reset_timestamp; }

std::string Game_Loop::get_executable_name() { return executable_name; }

std::shared_ptr<spdlog::logger> Game_Loop::get_logger() { return logger; }

void Game_Loop::register_on_initialize(std::function<void()> callback) {
  on_initialize = callback;
}

DWORD WINAPI mainThread(LPVOID lpParam) {
  game_loop.initialize_loop();
  while (true) {
    game_loop.on_loop();
    Sleep(1000);
  }

  return 0;
}
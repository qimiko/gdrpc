#include "gdapi.hpp"
#include "pch.h"

difficulty getDiffValue(int diff) {
  switch (diff) {
  case 10:
    return difficulty::easy;
  case 20:
    return difficulty::normal;
  case 30:
    return difficulty::hard;
  case 40:
    return difficulty::harder;
  case 50:
    return difficulty::insane;
  case 0:
  default:
    return difficulty::na;
  }
}

demon_difficulty getDemonDiffValue(int diff) {
  switch (diff) {
  case 3:
    return demon_difficulty::easy;
  case 4:
    return demon_difficulty::medium;
  case 5:
    return demon_difficulty::insane;
  case 6:
    return demon_difficulty::extreme;
  case 0:
  case 1:
  case 2:
  default:
    return demon_difficulty::hard;
  }
}

std::string getDifficultyName(GDlevel &level) {
  // for some reason auto/demon levels don't have a proper difficulty
  if (level.isAuto) {
    return "auto";
  }

  // this is also messy
  if (level.isDemon) {
    switch (level.demonDifficulty) {
    case demon_difficulty::easy:
      return "easy_demon";
    case demon_difficulty::medium:
      return "medium_demon";
    case demon_difficulty::insane:
      return "insane_demon";
    case demon_difficulty::extreme:
      return "extreme_demon";
    default:
    case demon_difficulty::hard:
      return "hard_demon";
    }
  }

  // definitely a better way to do this
  switch (level.difficulty) {
  case difficulty::easy:
    return "easy";
  case difficulty::normal:
    return "normal";
  case difficulty::hard:
    return "hard";
  case difficulty::harder:
    return "harder";
  case difficulty::insane:
    return "insane";
  case difficulty::na:
  default:
    return "na";
  }
}

std::string to_param_list(Params &params) {
  std::stringstream s;

  // can't use the normal for loop as we need more iterator control!!
  for (auto it = params.begin(); it != params.end(); ++it) {
    s << it->first << "=" << it->second;
    if (std::next(it) != params.end()) {
      s << "&";
    }
  }

  return s.str();
}

GD_Client::GD_Client(std::string host)
    : game_version(21), secret("Wmfd2893gb7"), host(host) {
  client = std::make_shared<httplib::Client>(host.c_str());
}

std::string GD_Client::post_request(const char *url, Params &params) {
  // this function has become magic dust for me lol
  params.emplace("gameVersion", std::to_string(game_version));
  params.emplace("secret", secret);

  auto res = client.get()->Post(url, params);

  auto body = res->body;
  if (body == "-1") {
    throw std::logic_error("post request failure");
  }

  return body;
}

bool GD_Client::get_user_info(int &accID, GDuser &user) {
  Params params({{"targetAccountID", std::to_string(accID)}});
  auto user_string = post_request("/database/getGJUserInfo20.php", params);

  try {
    auto user_map = to_robtop(user_string);

    user.name = user_map[1];
    user.ID = std::stoi(user_map[2], nullptr);
    user.accID = std::stoi(user_map[16], nullptr);
    return true;
  } catch (const std::exception &e) {
    // throw the exceptions that will actually have a message
    throw e;
  } catch (...) {
    user.name = "invalid";
    user.ID = -1;
    user.accID = -1;
  }
  return false;
}

bool GD_Client::get_player_info(int &playerID, GDuser &user) {
  Params params({{"str", std::to_string(playerID)}});
  auto player_string = post_request("/database/getGJUsers20.php", params);

  try {
    auto user_map = to_robtop(player_string);

    user.name = user_map[1];
    user.ID = std::stoi(user_map[2], nullptr);
    user.accID = std::stoi(user_map[16], nullptr);
    return true;
  } catch (const std::exception &e) {
    throw e;
  } catch (...) {
    user.name = "invalid";
    user.ID = -1;
    user.accID = -1;
  }
  return false;
}

bool GD_Client::get_user_rank(GDuser &user) {
  Params params(
      {{"type", "relative"}, {"accountID", std::to_string(user.accID)}});
  auto leaderboardString = post_request("/database/getGJScores20.php", params);

  auto leaderboard_list = explode(leaderboardString, '|');

  bool found_user = false;
  Robtop_Map seglist;

  if (leaderboard_list.size() >= 24) {
    seglist = to_robtop(leaderboard_list[24]);
    found_user = (std::stoi(seglist[16], nullptr) == user.accID);
  }

  if (!found_user) { // hey look its the checks
    // so basically you look for
    // :16:<id>:
    std::string lookup_string = ":16:" + std::to_string(user.accID) + ":";

    auto player_entry =
        std::find_if(leaderboard_list.begin(), leaderboard_list.end(),
                     [&lookup_string](std::string &entry) {
                       return (entry.find(lookup_string) != std::string::npos);
                     });

    if (player_entry == leaderboard_list.end()) {
      user.rank = -1;
      throw std::runtime_error("could not find player");
    }

    seglist = to_robtop(*player_entry);
  }

  user.rank = std::stoi(seglist[6], nullptr);
  return true;
}

// reads string at address
// does proper length checks
std::string readString(int *addr) {
  int length = *(int *)((int)addr + 0x10);
  if (length >= 16) {
    return std::string((char *)(*addr));
  } else {
    return std::string((char *)addr);
  }
}

// i was going to make a joke about this being the better thing like the id
// parsing but this is _really_ messy no error handling either. good luck
bool parseGJGameLevel(int *gameLevel, GDlevel &level) {
  // obj count at 0x1D8
  int newID = *(int *)((int)gameLevel + 0xF8);
  int levelLocation = *(int *)((int)gameLevel + 0x364);

  // don't calculate more than we have to, but the editor keeps id 0
  if (newID == level.levelID && levelLocation != 2) {
    return true;
  }

  level.levelID = newID;
  level.stars = *(int *)((int)gameLevel + 0x2AC);

  try {
    level.name = readString((int *)((int)gameLevel + 0xFC));
  } catch (...) {
    return false;
  }

  if (levelLocation == 1) {
    level.author = "RobTop"; // author is "" on these
    int diffValue = *(int *)((int)gameLevel + 0x1BC) * 10;
    if (diffValue == 60) {
      level.isDemon = true;
      level.difficulty = difficulty::insane;
      level.demonDifficulty = demon_difficulty::easy;
    } else {
      level.isDemon = false;
      level.difficulty = getDiffValue(diffValue);
    }
  } else {
    level.author = readString((int *)((int)gameLevel + 0x144));
    level.difficulty = getDiffValue(*(int *)((int)gameLevel + 0x1E4));

    // don't have the proper booleans mapped
    level.isDemon =
        (level.stars >= 10); // can't tell if this is more readable or not
    level.isAuto = (level.stars == 1);

    if (level.isDemon) {
      level.demonDifficulty =
          getDemonDiffValue(*(int *)((int)gameLevel + 0x2A0));
    }
  }
  return true;
}

Robtop_Map to_robtop(std::string &string, char delimiter) {
  std::stringstream segments(string);
  std::string previous_segment, current_segment;
  int position = 0;

  Robtop_Map robtop;

  auto split_string = explode(string, delimiter);

  for (auto it = split_string.begin(); it != split_string.end(); ++it) {
    // get position, check if odd (aka key)
    if ((it - split_string.begin()) % 2 == 0) {
      robtop.emplace(std::stoi(*it, nullptr), *std::next(it));
    }
  }

  return robtop;
}

// helper function
std::vector<std::string> explode(std::string &string, char separator) {
  std::stringstream segmentstream(string);
  std::string segmented;
  std::vector<std::string> splitlist;

  while (std::getline(segmentstream, segmented, separator)) {
    splitlist.push_back(segmented);
  }

  return splitlist;
}
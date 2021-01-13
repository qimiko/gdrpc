#include "gdapi.hpp"

Demon_Difficulty getDemonDiffValue(int diff) {
  switch (diff) {
  case 3:
    return Demon_Difficulty::Easy;
  case 4:
    return Demon_Difficulty::Medium;
  case 5:
    return Demon_Difficulty::Insane;
  case 6:
    return Demon_Difficulty::Extreme;
  case 0:
  case 1:
  case 2:
  default:
    return Demon_Difficulty::Hard;
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
    case Demon_Difficulty::Easy:
      return "easy_demon";
    case Demon_Difficulty::Medium:
      return "medium_demon";
    case Demon_Difficulty::Insane:
      return "insane_demon";
    case Demon_Difficulty::Extreme:
      return "extreme_demon";
    default:
    case Demon_Difficulty::Hard:
      return "hard_demon";
    }
  }

  // definitely a better way to do this
  switch (level.difficulty) {
  case Difficulty::Easy:
    return "easy";
  case Difficulty::Normal:
    return "normal";
  case Difficulty::Hard:
    return "hard";
  case Difficulty::Harder:
    return "harder";
  case Difficulty::Insane:
    return "insane";
  case Difficulty::Demon:
    return "hard_demon";
  case Difficulty::Na:
  default:
    return "na";
  }
}

GD_Client::GD_Client(std::string host, std::string prefix)
    : game_version(21), secret("Wmfd2893gb7"), host(host), prefix(prefix) {
  client = std::make_shared<httplib::Client>(host.c_str());
}

std::string GD_Client::post_request(std::string url, Params &params) {
  params.emplace("gameVersion", std::to_string(game_version));
  params.emplace("secret", secret);

  std::string full_url = prefix + url;

  auto res = client.get()->Post(full_url.c_str(), params);

  auto body = res->body;
  if (body == "-1") {
    throw std::logic_error("post request failure");
  }

  return body;
}

bool GD_Client::get_user_info(int &accID, GDuser &user) {
  Params params({{"targetAccountID", std::to_string(accID)}});
  auto user_string = post_request(urls.get_user_info, params);

  try {
    auto user_map = to_robtop(user_string);

    user.name = user_map.at(1);
    user.ID = std::stoi(user_map.at(2), nullptr);
    user.accID = std::stoi(user_map.at(16), nullptr);
    return true;
  } catch (const std::exception &e) {
    // throw the exceptions that will actually have a message
    throw e;
  }
  return false;
}

bool GD_Client::get_player_info(int &playerID, GDuser &user) {
  Params params({{"str", std::to_string(playerID)}});
  auto player_string = post_request(urls.get_users, params);

  try {
    auto user_map = to_robtop(player_string);

    user.name = user_map.at(1);
    user.ID = std::stoi(user_map.at(2), nullptr);
    user.accID = std::stoi(user_map.at(16), nullptr);
    return true;
  } catch (const std::exception &e) {
    throw e;
  }
  return false;
}

bool GD_Client::get_user_rank(GDuser &user) {
  Params params(
      {{"type", "relative"}, {"accountID", std::to_string(user.accID)}});
  auto leaderboardString = post_request(urls.get_scores, params);

  auto leaderboard_list = explode(leaderboardString, '|');

  bool found_user = false;
  Robtop_Map seglist;

  if (leaderboard_list.size() >= 24) {
    seglist = to_robtop(leaderboard_list.at(24));
    found_user = (std::stoi(seglist.at(16), nullptr) == user.accID);
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

  user.rank = std::stoi(seglist.at(6), nullptr);
  return true;
}

void GD_Client::set_urls(GDUrls new_urls) { urls = new_urls; }

bool parseGJGameLevel(GJGameLevel *in_memory, GDlevel &level) {
  auto newID = in_memory->levelID;
  auto levelLocation = in_memory->levelType;

  // don't calculate more than we have to, but the editor keeps id 0
  if (newID == level.levelID && levelLocation != GJLevelType::Editor) {
    return true;
  }

  level.levelID = newID;
  level.stars = in_memory->stars;

  level.name = in_memory->levelName;

  // good robtop security
  level.isDemon = static_cast<bool>(in_memory->demon);
  level.isAuto = in_memory->autoLevel;

  if (levelLocation == 1) {
    level.author = "RobTop"; // author is "" on these
    level.difficulty = static_cast<Difficulty>(in_memory->difficulty);

    if (level.difficulty == Difficulty::Demon) {
      level.demonDifficulty = Demon_Difficulty::Easy;
    }
  } else {
    level.author = in_memory->userName;
    level.difficulty = static_cast<Difficulty>(in_memory->ratingsSum / 10);

    if (level.isDemon) {
      level.demonDifficulty = getDemonDiffValue(in_memory->demonDifficulty);
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
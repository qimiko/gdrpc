#pragma once
#ifndef GDAPI_H
#define GDAPI_H
#include "pch.h"
#include <algorithm>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <httplib.h>

enum class difficulty {
  na,
  easy,
  normal,
  hard,
  harder,
  insane,
}; // note that no demon difficulty exists

enum class demon_difficulty { none, easy, medium, hard, insane, extreme };

// thinking about this a bit later
// probably shouldn't have used structs lol

struct GDlevel {
  int levelID = -1;
  std::string name;
  std::string author = "-";
  int authorID = -1;
  int stars = 0;
  difficulty difficulty = difficulty::na;
  demon_difficulty demonDifficulty = demon_difficulty::none;
  bool isAuto = false;
  bool isDemon = false;
}; // this is a really barebones struct btw

struct GDuser {
  int ID = -1;
  int accID = -1;
  std::string name;
  int rank = -1;
};

difficulty getDiffValue(int diff);
demon_difficulty getDemonDiffValue(int diff);
std::string getDifficultyName(GDlevel &level);

typedef std::multimap<std::string, std::string> Params;
typedef std::unordered_map<int, std::string> Robtop_Map;

class GD_Client {
private:
  std::string host;

  const int game_version;
  const std::string secret;

  std::shared_ptr<httplib::Client> client;

  // makes an internet post request to boomlings.com
  // returns 0 if succeed
  std::string post_request(const char *, Params &);
public:
  GD_Client(std::string host = "boomlings.com");

  bool get_user_info(int &accID, GDuser &user);
  bool get_player_info(int &playerID, GDuser &user);

  bool get_user_rank(GDuser &user);
};

Robtop_Map to_robtop(std::string &, char delimiter = ':');

// splits a string by substring, much like in other languages
std::vector<std::string> explode(std::string &string, char separator);

bool parseGJGameLevel(int *gameLevel, GDlevel &level);

#endif // !GDAPI_H
#pragma once
#ifndef GDAPI_H
#define GDAPI_H
#include "pch.h"
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <unordered_map>
#include <wininet.h>

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

typedef std::unordered_map<std::string, std::string> Params;
// makes an internet post request to boomlings.com
// returns 0 if succeed
DWORD post_request(const char *, Params &, std::string &);

bool getUserInfo(int &accID, GDuser &user);
bool getPlayerInfo(int &playerID, GDuser &user);

bool getUserRank(GDuser &user);

bool parseGJGameLevel(int *gameLevel, GDlevel &level);

typedef std::unordered_map<int, std::string> Robtop_Map;

Robtop_Map to_robtop(std::string &, char delimiter = ':');

// splits a string by substring, much like in other languages
std::vector<std::string> explode(std::string & string, char separator);

#endif // !GDAPI_H
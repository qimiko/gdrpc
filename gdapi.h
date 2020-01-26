#pragma once
#ifndef GDAPI_H
#define GDAPI_H
#include "pch.h"
#include <sstream>
#include <tchar.h>
#include <string>
#include <WinInet.h>
#include <vector>

enum class difficulty {
  na,
  easy,
  normal,
  hard,
  harder,
  insane,
}; // note that no demon difficulty exists

enum class demon_difficulty { 
	none,
	easy,
	medium,
	hard,
	insane,
	extreme
};

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
std::string getDifficultyName(GDlevel& level);

// makes an internet post request to boomlings.com
// returns 0 if succeed
DWORD makeRequest(std::string data, LPCWSTR url, std::string& response);

bool getUserInfo(int playerID, GDuser& user);
bool getUserRank(GDuser& user);
bool getLevel(int levelid, GDlevel& level);

void getOfficialInfo(int id, GDlevel& level);

// splits a string by substring, much like in other languages
std::vector<std::string> explode(std::string& string, char separator);

#endif  // !GDAPI_H

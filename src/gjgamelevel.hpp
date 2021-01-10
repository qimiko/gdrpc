#pragma once
#ifndef GJGAMELEVEL_HPP
#define GJGAMELEVEL_HPP
#include <string>

enum GJLevelType { Local = 1, Editor = 2, Saved = 3 };

struct GJGameLevel {
  int8_t padding_0[0xEC];

  void *lastBuildSave;
  int levelID_rand; // levelID + seed = levelID_rand
  int levelID_seed;
  int levelID;
  std::string levelName;
  std::string levelDesc;
  std::string levelString;
  std::string userName;
  std::string recordString;

  std::string uploadDate;
  std::string updateDate;

  int userID_rand;
  int userID_seed;
  int userID;

  int accountID_rand;
  int accountID_seed;
  int accountID;

  int difficulty;
  int audioTrack;
  int songID;

  int levelRev;

  bool unlisted;

  int objectCount_rand;
  int objectCount_seed;
  int objectCount;
  int averageDifficulty;
  int ratings;
  int ratingsSum;

  int downloads;

  bool isEditable;
  bool gauntletLevel;
  bool unknown_bool_1;

  int workingTime;
  int workingTime2;

  bool lowDetailMode;
  bool unknown_bool_2;

  int isVerified_rand;
  int isVerified_seed;
  bool isVerified;
  bool isUploaded;
  bool hasBeenModified;

  int levelVersion;
  int gameVersion;

  int attempts_rand;
  int attempts_seed;
  int attempts;

  int jumps_rand;
  int jumps_seed;
  int jumps;

  int clicks_rand;
  int clicks_seed;
  int clicks;

  int attemptTime_rand;
  int attemptTime_seed;
  int attemptTimd;

  int chk;

  bool unknown_bool_3;

  int normalPercent; // yes, it is out of order
  int normalPercent_seed;
  int normalPercent_rand;

  int orbCompletion_rand;
  int orbCompletion_seed;
  int orbCompletion;

  int newNormalPercent2_rand;
  int newNormalPercent2_seed;
  int newNormalPercent2;

  int practicePercent;

  int likes;
  int dislikes;
  int levelLength;
  int featured;

  bool isEpic;
  bool unknown_bool_4;
  int levelFolder;

  int dailyID_rand;
  int dailyID_seed;
  int dailyID;

  int demon_rand;
  int demon_seed;
  int demon;
  int demonDifficulty;
  int stars_rand;
  int stars_seed;
  int stars;

  bool autoLevel;
  int coins;
  int coinsVerified_rand;
  int coinsVerified_seed;
  int coinsVerified;

  int password_rand;
  int password_seed;

  int originalLevel_rand;
  int originalLevel_seed;
  int originalLevel;

  bool twoPlayerMode;

  int unknown_int_1;

  int firstCoinVerified_rand;
  int firstCoinVerified_seed;
  int firstCoinVerified;

  int secondCoinVerified_rand;
  int secondCoinVerified_seed;
  int secondCoinVerified;

  int thirdCoinVerified_rand;
  int thirdCoinVerified_seed;
  int thirdCoinVerified;

  int starsRequested;

  int unknown_int_2;
  int unknown_int_3;
  int unknown_int_4;
  int unknown_int_5;
  int unknown_int_6;
  int unknown_int_7;
  int unknown_int_8;
  int unknown_int_9;

  std::string unknown_string_1;

  bool unknown_bool_5;
  bool unknown_bool_6;

  int unknown_int_10;
  bool unknown_bool_7;

  void *lastCameraPos;

  int unknown_int_11;

  float lastEditorZoom;
  int lastBuildTab;
  int lastBuildPage;
  int lastBuildGroupID;

  GJLevelType levelType;

  int unknown_int_12;
  std::string tempName;
  std::string capacityString;

  bool unknown_bool_8;
  std::string personalBests;
};

#endif

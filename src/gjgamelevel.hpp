#pragma once
#ifndef GJGAMELEVEL_HPP
#define GJGAMELEVEL_HPP
#include <string>

enum GJLevelType { Local = 1, Editor = 2, Saved = 3 };

struct GJGameLevel {
  int8_t padding_0[0xE8];
  void *lastBuildSave; // this is actually a CCDictionary *
  int levelID;
  std::string levelName;
  std::string levelDesc;
  std::string levelString;
  std::string userName;
  std::string recordString;
  std::string uploadDate;
  std::string updateDate;
  int userID;
  int difficulty;
  int audio_track;
  int song_id;
  int levelRev;
  int objectCount;
  int order;
  int ratings;
  int ratingsSum;
  int downloads;
  int completes;
  bool isEditable;
  bool isVerified;
  bool isUploaded;
  bool hasBeenModified;
  int levelVersion;
  int gameVersion;
  int attempts;
  int jumps;
  int normalPercent;
  int practicePercent;
  int likes;
  int dislikes;
  int levelLength;
  int featured;
  bool demon;
  int stars;
  bool autoLevel;
  int coins;
  int password;
  int originalLevel;
  bool twoPlayerMode;
  int failedPasswordAttempts;
  bool showedSongWarning;
  int starRatings;
  int starRatingsSum;
  int maxStarRatings;
  int minStarRatings;
  int demonVotes;
  int rateStars;
  int rateFeature;
  std::string rateUser;
  bool dontSave;
  int requiredCoins;
  bool isUnlocked;
  void *lastCameraPos; // CCPoint *
  int unused;
  float lastEditorZoom;
  int lastBuildTab;
  int lastBuildPage;
  int lastBuildGroupID;
  GJLevelType levelType;
  int M_ID;
  std::string tempName;
  int capacity001;
  int capacity002;
  int capacity003;
  int capacity004;
};

#endif

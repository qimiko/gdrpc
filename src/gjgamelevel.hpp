#pragma once
#ifndef GJGAMELEVEL_HPP
#define GJGAMELEVEL_HPP

enum GJLevelType { Local = 1, Editor = 2, Saved = 3 };

struct GJGameLevel {
  int8_t padding_0[0xF8];

  int id;
  std::string name;

  int8_t padding_1[0x30];

  std::string author;

  int8_t padding_2[0x60];

  int main_difficulty;
	int audio_track;
	int song_id;

	int8_t padding_3[0x10];

	int object_count;
	int order;
	int ratings;
  int difficulty;

  int8_t padding_7[0x60];

  int current_best;

  int8_t padding_4[0x54];

  int demon_difficulty;

  int8_t padding_5[0x8];

  int stars;

  int8_t padding_6[0xB4];

  GJLevelType level_type;
};

#endif

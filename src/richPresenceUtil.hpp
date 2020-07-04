#pragma once
#include "DRPWrap.hpp"
#include "gdapi.hpp"
#include "pch.h"
#include <algorithm>
#include <ctime>
#include <sstream>
#include <stdio.h>

#include <fstream>
#include <toml.hpp>

#include <fmt/format.h>

#ifndef RICHPRESENCEUTIL_H
#define RICHPRESENCEUTIL_H
enum class playerState {
  level,
  editor,
  menu,
};

extern playerState currentPlayerState;
extern int *currentGameLevel;
extern bool updatePresence;
extern bool updateTimestamp;
extern bool editor_reset_timestamp;
void safeClose();
DWORD WINAPI mainThread(LPVOID lpParam);
#endif
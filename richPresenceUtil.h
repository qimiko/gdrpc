#pragma once
#include "pch.h"
#include <stdio.h>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <sstream>
#include "DRPWrap.h"
#include "gdapi.h"

#include "libraries/toml11/toml.hpp"
#include <fstream>

#define FMT_HEADER_ONLY
#include "libraries/fmt/include/fmt/format.h"

#ifndef RICHPRESENCEUTIL_H
#define RICHPRESENCEUTIL_H
enum class playerState
{
	level,
	editor,
	menu,
};

extern playerState currentPlayerState;
extern int *currentGameLevel;
extern bool updatePresence;
extern bool updateTimestamp;

void safeClose();
DWORD WINAPI mainThread(LPVOID lpParam);
#endif

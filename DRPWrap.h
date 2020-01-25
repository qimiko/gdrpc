#pragma once
#pragma comment(lib, "discord-rpc.lib")

#include <time.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include "discord_files/discord_rpc.h"
#include "DiscordSecret.h"

#ifndef DRPWRAP
#define DRPWRAP

namespace DRP {
	int getPresenceStatus();
	void UpdatePresence(const char* details, const char* largeText,
		const char* smallText, const char* statetext,
		const char* smallImage, time_t timestamp);
	void InitDiscord();
}
#endif
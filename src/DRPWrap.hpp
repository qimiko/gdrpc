#pragma once
#pragma comment(lib, "discord-rpc.lib")

#include "DiscordSecret.hpp"
#include <cstdint>
#include <cstdlib>
#include <discord_rpc.h>
#include <iostream>
#include <time.h>

#ifndef DRPWRAP
#define DRPWRAP

namespace DRP {
int getPresenceStatus();
void UpdatePresence(const char *details, const char *largeText,
                    const char *smallText, const char *statetext,
                    const char *smallImage, time_t timestamp);
void InitDiscord();
} // namespace DRP
#endif